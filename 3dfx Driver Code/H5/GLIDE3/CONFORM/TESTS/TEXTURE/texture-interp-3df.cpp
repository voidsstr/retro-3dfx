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
#include <stddef.h>

#include "conform.h"
#include "vertex.h"
#include "vmglide.h"
#include "matrix.h"

static char *test_name = "texture-interp-3df";
static char *test_description = 
 "Decal and iterated RGB textures from a .3df file applied to rotated triangles. "
 "Rotated decal and interated textures applied to triangles."
 "All with and without AA, with and without bilinear filtering.";

#define NUM_ROTS  17 // preferred number of rotations of each prim

#define SCALE_TWEAK .35f // scale down rotated prims to keep them in their 1x1 bbox

#define CLIP_COORDS_W_VALUE 3.0f // arbitrary

#define MAX_VERTS 3

#define NUM_PERMUTATIONS 8

// 1) rotated tris, decal texture applied
// 2) tris, rotated decal texture applied
// 3) rotated tris, texture * iterated  RGB applied.
// 4) tris, rotated texture * iterated  RGB applied.

// 5) rotated tris, one texture vertex having a different depth than the others
// 6) tris, rotated textures w/one texture vertex having a different depth 
//    than the others
// 7) rotated tris, texture * iterated RGB applied, with one texture vertex
//    having a different depth than  the others.
// 9) tris, rotated texture * iterated RGB applied, with one texture vertex
//    having a different depth than  the others.

// AA command line option.
// Bilinear Filtering command line option.

typedef struct {
  Vertex verts[MAX_VERTS]; // pointer to all of our primitive verts
  int num_verts;       // number of vertices currently in use
  int num_rotations;   // number of rotations of each prim per image
  int cur_rep;         // the current call to do_conform()
  float angle_incr;    // amount to incremement each rotation
  float scale_x;       // amount to scale each rotation by in x
  float scale_y;       // amount to scale each rotation by in y
  float offset_x;       // amount to offset each successive rotation in x
  float offset_y;       // amount to offset each successive rotation in y
  int use_window_coords; // use window coords instead of clip coords
  int do_aa;           // enable anti-aliasing
  int do_bilinear_filtering; // enable bilinear filtering/mipmapping
  int flags;           // current test permutation
  float color[MAX_VERTS][3]; 
} test_data;

#ifndef BIT
#define BIT(n)  (1UL<<(n))
#endif

#define DO_ITER_RGB       BIT(0)
#define DO_ROTATE_TEXTURE BIT(1)
#define DO_TWEAK_DEPTH    BIT(2)

/************************************************************************/
/************************************************************************/
/**             Here begins the texture utility stuff.                 **/
/************************************************************************/
/************************************************************************/

// From tlib.h
typedef FxU32 TlPalette[256];

typedef struct {
  FxU8  yRGB[16];
  FxI16 iRGB[4][3];
  FxI16 qRGB[4][3];
  FxU32 packed_data[12];
} TlNCCTable;

typedef union {
    TlPalette  palette;
    TlNCCTable nccTable;
} TlTextureTable;

typedef struct {
    GrTexInfo      info;
    GrTexTable_t   tableType;
    TlTextureTable tableData;
} TlTexture;
#define NO_TABLE ((GrTexTable_t)(~0))

// stolen from tlib.c
static GrTexTable_t tex_table_type( GrTextureFormat_t format ) {
    GrTexTable_t rv = (GrTexTable_t)NO_TABLE;
    switch( format ) {
    case GR_TEXFMT_YIQ_422:
    case GR_TEXFMT_AYIQ_8422:
        rv = GR_TEXTABLE_NCC0;
        break;
    case GR_TEXFMT_P_8:
    case GR_TEXFMT_AP_88:
        rv = GR_TEXTABLE_PALETTE;
        break;
    }
    return rv;
}


/* so we don't have to worry about which OS we are on - djh */
#if defined(__WIN32__) || defined(__DOS32__)
#define GD_FILE_SEPARATOR_STRING "\\"
#define GD_CURRENT_DIRECTORY_STRING ".\\"
#elif macintosh
#define GD_FILE_SEPARATOR_STRING ":"
#define GD_CURRENT_DIRECTORY_STRING ":"
#elif __unix__
#define GD_FILE_SEPARATOR_STRING "/"
#define GD_CURRENT_DIRECTORY_STRING "./"
#endif
static char *_gd_load_path = NULL;

/*-------------------------------------------------------------------
  Function: gdSetLoadPath
  Date: 10/13/96
  Implementor(s): mlwp
  Library: AT Utility
  Description: 
    Set ATB's load path
  Arguments:
    load_path  -   new load path
  Return:
    nothing
  -------------------------------------------------------------------*/

void gdSetLoadPath(const char *load_path) {
    int len ;

    if ( load_path ) {

        len = strlen(load_path);

        if ( _gd_load_path == NULL )
            _gd_load_path = (char *)malloc(len+1);
        else _gd_load_path = (char *)realloc(_gd_load_path, len+1);
    
        if ( _gd_load_path == NULL ) {
            fprintf( stderr, "gdLoadPath: No memory\n" );
            return ;
        }

        strcpy(_gd_load_path, load_path);
    } else _gd_load_path = NULL;
}

/*-------------------------------------------------------------------
  Function: gd:FileLocate
  Date: 10/13/96
  Implementor(s): mlwp
  Library: AT Utility
  Description: 
    Try to open the file specified by filename, if this fails and there is
    a search path try the specified directories.
  Arguments:
    filename  -   base name of file
    full_path -   buffer for expanded file path
  Return:
    expanded file path if succesful, NULL if could not find path
  -------------------------------------------------------------------*/

char *
gdFileLocate(const char *filename, char *full_path) {
    char  tmp_path[512];
    char *partial_path;
    FILE *f;

    strcpy(full_path, filename);
    if( ( f = fopen( full_path, "r" ) ) != NULL ) {
         fclose(f);
         return full_path;
    } 

    if ( _gd_load_path == NULL ) {
        gdSetLoadPath(getenv("GD_LOAD_PATH"));
    }

    if ( _gd_load_path != NULL ) {
        strcpy( tmp_path, _gd_load_path );
        for ( partial_path = strtok( tmp_path, ";" );
             partial_path != 0 ;
             partial_path = strtok( NULL, ";" ) ) {
             #if macintosh
             /* it is hard to know what directory fopen will start in */
             /* so we need to give it a full path, starting with app_path */
             extern char app_path[];

             strcpy(full_path, app_path);

             /* if the partial_path is not the current directory, append it */
             if(strlen(partial_path) != 1 || partial_path[0] != ':') {
                 strcat(full_path, partial_path);
             }
             #else
             strcpy( full_path, partial_path );
             #endif
             strcat( full_path, GD_FILE_SEPARATOR_STRING );
             strcat( full_path, filename );
             if( ( f = fopen( full_path, "r" ) ) != NULL ) {
                 fclose(f);
                 return full_path;
             }
        }
    } 

    return NULL;
}
/*-------------------------------------------------------------------
  Function: tlload_texture (stolen from tlib.c)
  Date: 3/3
  Implementor(s): jdt
  Library: Test Libarary
  Description:
  Load Texture
  
  This example loads textures from a .3df file.  3DF files
  containe pre-computed mipmaps data along with any
  necessary supplementary information, for example 
  palettes, ncc-tables, level-of-detail description,
  aspect ratio or format 
  
  The gu3dfGetInfo and gu3dfLoad APIs load A 3DF file
  into Gu3DfInfo structure from a file on disk.  Data
  can then be extracted from the gu3DfInfo structure 
  to initialize a GrTexInfo structure used in the 
  glide texturing routines.  Also note that texture table
  ( either NCC or Palette ) management is left up to the
  application programmer.  
  Arguments:
  filename - name of .3df file on disk
  info - Pointer to GrTexInfo
  tableType - pointer to tabletype
  table - pointer to table data
  Return:
  0 - fail
  1 - pass
  -------------------------------------------------------------------*/

int
load_texture( const char *filename, 
             GrTexInfo *info, 
             GrTexTable_t *tableType,
             void *table ) {
    Gu3dfInfo tdfInfo;
    int rv = 0;
    char fullPath[512];

    //assert( filename );
    //assert( info );
    //assert( tableType );
    //assert( table );

    if ( gdFileLocate(filename, fullPath) == NULL ) {
       return NULL;
    }

    if ( gu3dfGetInfo( fullPath, &tdfInfo ) ) {
        tdfInfo.data = malloc( tdfInfo.mem_required );
        //assert( tdfInfo.data );
        if ( gu3dfLoad( fullPath, &tdfInfo ) ) {
            info->smallLodLog2    = tdfInfo.header.small_lod;
            info->largeLodLog2    = tdfInfo.header.large_lod;
            info->aspectRatioLog2 = tdfInfo.header.aspect_ratio;
            info->format      = tdfInfo.header.format;
            info->data        = tdfInfo.data;
            *tableType = tex_table_type( info->format );
            switch( *tableType ) {
            case GR_TEXTABLE_NCC0:
            case GR_TEXTABLE_NCC1:
            case GR_TEXTABLE_PALETTE:
                memcpy( table, &(tdfInfo.table), sizeof( TlTextureTable ) );
                break;
            default:
                break;
            }
            rv = 1;
        }
    }
    return rv;
}

/************************************************************************/
/************************************************************************/
/**             Here ends the texture utility stuff.                   **/
/************************************************************************/
/************************************************************************/

 
/*---------------------------------------------------------------------------
@func rot_vertx_z
@arg test_data *local_ptr - the local test data
@arg Vertex *out_verts - where to return the xformed verts
@arg Vertex *in_verts - the source verts
@html
 Rotate our vertices around the Z-axis. 
@end
--------------------------------------------------------------------------*/
void rot_verts_z(test_data *local_ptr,
                 Vertex *out_verts, Vertex *in_verts) 
{
    int num_verts = local_ptr->num_verts;
    float z_degrees = local_ptr->cur_rep * local_ptr->angle_incr;
    float scale_x = local_ptr->scale_x;
    float scale_y = local_ptr->scale_y;
    float dx = local_ptr->offset_x;
    float dy = local_ptr->offset_y;
    int dont_normalize_colors = local_ptr->use_window_coords;

    matrix rotm;       
    mat_make_z_rot(rotm, DEG2RAD(z_degrees));
    float color_divisor = 255.0f;
    if (dont_normalize_colors) {
      color_divisor = 1.0f;
    }

    for(int i = 0; i < num_verts; i++) {
      mat_vertex_mul(&out_verts[i], &in_verts[i], rotm);
      out_verts[i].x *= scale_x * SCALE_TWEAK;
      out_verts[i].y *= scale_y * SCALE_TWEAK;
      out_verts[i].y += dy;
      out_verts[i].x += dx; 

      out_verts[i].tmu[0].r = local_ptr->color[i][0]/color_divisor;
      out_verts[i].tmu[0].g = local_ptr->color[i][1]/color_divisor;
      out_verts[i].tmu[0].b = local_ptr->color[i][2]/color_divisor; 
   }
}

/*---------------------------------------------------------------------------
@func rot_tex_vertx_z
@arg test_data *local_ptr - the local test data
@arg Vertex *out_verts - where to return the xformed verts
@arg Vertex *in_verts - the source verts
@html
 Rotate our texture vertices around the Z-axis. Scale and add
 an offset to the primitive vertices. 
 NOTE: This assumes texture coords are from 0 to 255
@end
--------------------------------------------------------------------------*/
void rot_tex_verts_z(test_data *local_ptr,
                     Vertex *out_verts, Vertex *in_verts)
{
    int num_verts = local_ptr->num_verts;
    float z_degrees = local_ptr->cur_rep * local_ptr->angle_incr;
    float scale_x = local_ptr->scale_x;
    float scale_y = local_ptr->scale_y;
    float dx = local_ptr->offset_x;
    float dy = local_ptr->offset_y;
    int dont_normalize_colors = local_ptr->use_window_coords;

    matrix rotm;       
    mat_make_z_rot(rotm, DEG2RAD(z_degrees));
    Vertex tmp_in, tmp_out;

    float color_divisor = 255.0f;
    float half = 0.5f;
    if (dont_normalize_colors) {
      color_divisor = 1.0f;  
      half = 127.5f;
    }

    for( int i = 0; i < num_verts; i++ ) {
      out_verts[i] = in_verts[i]; // copy over Alpha values, etc
      out_verts[i].x *= scale_x * SCALE_TWEAK;
      out_verts[i].y *= scale_y * SCALE_TWEAK;
      out_verts[i].y += dy;
      out_verts[i].x += dx; 

      out_verts[i].tmu[0].r = local_ptr->color[i][0]/color_divisor;
      out_verts[i].tmu[0].g = local_ptr->color[i][1]/color_divisor;
      out_verts[i].tmu[0].b = local_ptr->color[i][2]/color_divisor; 

      // normalize, rotate, and re-scale the texture coords
      tmp_in.x = (in_verts[i].tmu[0].s- half)/half;
      tmp_in.y = (in_verts[i].tmu[0].t- half)/half;

      mat_vertex_mul(&tmp_out, &tmp_in, rotm);            

      out_verts[i].tmu[0].s= tmp_out.x * half + half;
      out_verts[i].tmu[0].t= tmp_out.y * half + half;
    }
}


/**
 * Debug routine. Dump our verts to stdout.
 */
void dump_verts(int num_verts, Vertex *verts) {
  for (int i = 0; i < num_verts; i++) {
    printf("  [%d](x,y,w) = (%6.3f, %6.3f, %6.3f)\n", i,
            verts[i].x,
            verts[i].y,
            verts[i].w);
  }
}

/**
 * Initialize our triangle vertices.
 * @param td -  a pointer to our local_data structure (which holds
 *              the vertices we'll be init'ing)
 */
void init_verts(conform_state *state) {

  float divisor;
  test_data *local_ptr = (test_data *)state->local_data;

  // Specify a vertex layout
  vmgrVertexLayout(state, GR_PARAM_XY, offsetof(Vertex, x), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_A, offsetof(Vertex, tmu[0].a), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_W, offsetof(Vertex, w), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_ST0, offsetof(Vertex, tmu[0].s), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_Q0, offsetof(Vertex, tmu[0].q), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_RGB, offsetof(Vertex, tmu[0].r),  GR_PARAM_ENABLE);  
  if (local_ptr->use_window_coords) {
    divisor = 1.0f;
  } else {
    divisor = 255.0f;
  }

  Vertex *verts =  local_ptr->verts;
  local_ptr->num_verts = 3;

  verts[0].x = -1.0f; verts[0].y = -1.0f; verts[0].q = 1.0f;
  verts[1].x =  0.0f; verts[1].y =  1.0f; verts[1].q = 1.0f;
  verts[2].x =  1.0f; verts[2].y = -1.0f; verts[2].q = 1.0f;

  verts[0].tmu[0].a = 255.0f/divisor;  verts[0].w = CLIP_COORDS_W_VALUE;
  verts[1].tmu[0].a = 10.0f/divisor;   verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].tmu[0].a = 255.0f/divisor;  verts[2].w = CLIP_COORDS_W_VALUE;

  verts[0].tmu[0].s =   0.0f/divisor; 
  verts[0].tmu[0].t = 255.0f/divisor;

  verts[1].tmu[0].s = 127.5f/divisor; 
  verts[1].tmu[0].t =   0.0f/divisor;

  verts[2].tmu[0].s = 255.0f/divisor; 
  verts[2].tmu[0].t = 255.0f/divisor;

  verts[0].tmu[0].q = verts[1].tmu[0].q = verts[2].tmu[0].q = 1.0f;
}


int init_conform(conform_state *state, int local_argc, char **local_argv)
{
  test_data *local_ptr;
  int num_arg;

  vmgrInit(state); // Initialize self checking code

  state->test_name = test_name;
  state->test_description = test_description;

  if((state->local_data = calloc(1, sizeof(test_data))) == NULL) {
    // Could not allocate data for local memory
    log_perror(state,"Could not allocate memory for local data");
    return(0);
  } 

  // Set some default local state
  local_ptr = (test_data *)state->local_data;
  local_ptr->angle_incr = 0.0f;
  local_ptr->num_rotations = NUM_ROTS;
  local_ptr->cur_rep = 0;
  local_ptr->num_verts = 0;
  local_ptr->scale_x = 1.0f; 
  local_ptr->scale_y = 1.0f;
  local_ptr->offset_x = 0.25f;
  local_ptr->offset_y = 0.25f;
  local_ptr->use_window_coords = 0; // use clip coords by default
  local_ptr->do_aa = 0; // aa off
  local_ptr->do_bilinear_filtering = 0;
  local_ptr->flags = 0;

  // print out test dependent args
  num_arg = 0;
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    if(!_stricmp(*local_argv,"-nr") || // number of rotations
       !_stricmp(*local_argv,"/nr")) {
      ++local_argv;
      ++num_arg;
      if(!sscanf(*local_argv,"%d",&(local_ptr->num_rotations))) {
        // bad argument
      } 
      --local_argc;
    } else if(!_stricmp(*local_argv,"-aa") || // anti aliasing 
              !_stricmp(*local_argv,"/aa")) {
      local_ptr->do_aa = 1;
    } else if(!_stricmp(*local_argv,"-wc") || // use window coords
              !_stricmp(*local_argv,"/wc")) {
      local_ptr->use_window_coords = 1;
    } else if(!_stricmp(*local_argv,"-bf") || // use bilinear filtering & mipmapping
              !_stricmp(*local_argv,"/bf")) {
      local_ptr->do_bilinear_filtering = 1;
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
 
  // default state is cycle through all of our prims
  if (state->num_reps == 1) {
      state->num_reps = local_ptr->num_rotations * NUM_PERMUTATIONS;
  }

  // set a small conform window, if none was requested
  if ((state->conform_width == -1) &&
      (state->conform_height == -1)) {
    state->conform_width = 50;
    state->conform_height = 50;
  }

  // compute the amount we'll rotate successive frames.  We want
  // to cover a full rotation per rotation type.
  local_ptr->angle_incr = 360.0f/local_ptr->num_rotations;

  // setup image file names
  // this is manditory
  char tmp_str[128];
  sprintf(tmp_str,"%s%s%s", 
          test_name,
          (local_ptr->use_window_coords? "-wc" : ""),
          (local_ptr->do_aa? "-aa" : ""));

  frame_name(tmp_str,state);
  img_file_type(IMG_P6,state);

  return(-1); }

void close_conform(conform_state *state)
{
  // Free up local data
  free(state->local_data);
}

// 1) rotated tris, decal texture applied
// 2) tris, rotated decal texture applied

// 3) rotated tris, texture * iterated  RGB applied.
// 4) tris, rotated texture * iterated  RGB applied.

// 5) rotated tris, one texture vertex having a different depth than the others
// 6) tris, rotated textures w/one texture vertex having a different depth 
//    than the others

// 7) rotated tris, texture * iterated RGB applied, with one texture vertex
//    having a different depth than  the others.
// 8) tris, rotated texture * iterated RGB applied, with one texture vertex
//    having a different depth than  the others.

// AA command line option.
// Bilinear Filtering command line option.


void set_mode(conform_state *state, test_data *local_ptr)
{
  // Load texture data into system ram 
  TlTexture  texture;

  gdSetLoadPath(getenv("GD_LOAD_PATH"));

  load_texture("grid.3df", 
              &texture.info, 
              &texture.tableType, 
              &texture.tableData);

  // Download texture data to TMU 
  grTexDownloadMipMap(GR_TMU0,
                      grTexMinAddress(GR_TMU0),
                      GR_MIPMAPLEVELMASK_BOTH,
                      &texture.info);
  if ( texture.tableType != NO_TABLE ) {
      grTexDownloadTable( texture.tableType,
                          &texture.tableData);
  }


  if (local_ptr->flags & DO_ITER_RGB) {
    // interpolated RGB
    grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                   GR_COMBINE_FACTOR_LOCAL,
                   GR_COMBINE_LOCAL_ITERATED,
                   GR_COMBINE_OTHER_TEXTURE,
                   FXFALSE);
  } else {
    // decal
    grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                   GR_COMBINE_FACTOR_ONE,
                   GR_COMBINE_LOCAL_NONE,  
                   GR_COMBINE_OTHER_TEXTURE,
                   FXFALSE);        
  }

  grTexCombine(GR_TMU0,
               GR_COMBINE_FUNCTION_LOCAL,
               GR_COMBINE_FACTOR_NONE,
               GR_COMBINE_FUNCTION_NONE,
               GR_COMBINE_FACTOR_NONE,
               FXFALSE, FXFALSE );

  // Select Texture As Source of all texturing operations 
  grTexSource(GR_TMU0,
              grTexMinAddress(GR_TMU0),
              GR_MIPMAPLEVELMASK_BOTH,
              &texture.info );

  // Let the texture wrap in both directions
  grTexClampMode( GR_TMU0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

  // Turn on AA, if applicable.
  if (local_ptr->do_aa) {
    grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
                   GR_COMBINE_FACTOR_NONE,
                   GR_COMBINE_LOCAL_ITERATED,
                   GR_COMBINE_OTHER_NONE,
                   FXFALSE );
    grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                         GR_BLEND_ZERO, GR_BLEND_ZERO );
    grEnable(GR_AA_ORDERED);
  } else {
    grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                   GR_COMBINE_FACTOR_ONE,
                   GR_COMBINE_LOCAL_NONE,
                   GR_COMBINE_OTHER_CONSTANT,
                   FXFALSE);
    grAlphaBlendFunction(GR_BLEND_ONE , GR_BLEND_ZERO, 
                         GR_BLEND_ONE, GR_BLEND_ZERO);
    grDisable(GR_AA_ORDERED);
  }

  // turn on bilinear filtering, if applicable
  if (local_ptr->do_bilinear_filtering) {
    /* Enable Bilinear Filtering + Mipmapping */
    grTexFilterMode(GR_TMU0,
                    GR_TEXTUREFILTER_BILINEAR,
                    GR_TEXTUREFILTER_BILINEAR );
    grTexMipMapMode(GR_TMU0,
                    GR_MIPMAP_NEAREST,
                    FXFALSE );
    log_message(state, "Enabling bilinear filtering & mipmapping");
  }

}


int do_conform(conform_state *state)
{
  test_data *local_ptr;
  FxFloat    vnear = 0.0f, vfar = 1.0f;
  int v = 0;

  local_ptr = (test_data *)state->local_data;

  // First, clear the whole window
  vmgrClipWindow(state, 0, 0, state->screen_width, state->screen_height);
  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grBufferClear(0, 0, 0);
  grConstantColorValue(rgbacolor(state, 0x0000ffff));

  // Now set the clip window to just the check_region. (which can be bigger
  // than the conform window).
  vmgrClipWindow(state,
                 state->check_region.min_x,
                 state->check_region.min_y,
                 state->check_region.max_x,
                 state->check_region.max_y);
 
  if (!local_ptr->use_window_coords) {
    vmgrCoordinateSpace(state, GR_CLIP_COORDS);
    vmgrDepthRange( state, vnear, vfar );
  }

  vmgrViewport(state, state->conform_x,state->conform_y, 
              state->conform_width, state->conform_height);

  init_verts(state);

  local_ptr->flags = 0; // reset flags

  switch ((local_ptr->cur_rep / NUM_ROTS) % NUM_PERMUTATIONS) {
    case 0:
      // decal texture
      // rotated tris
      // constant depth
      break;
    case 1:
      // decal texture
      // rotated textures
      // constant depth
      local_ptr->flags = DO_ROTATE_TEXTURE;
      break;
    case 2:
      // iterated RGB * texture
      // rotated tris
      // constant depth
      local_ptr->flags = DO_ITER_RGB;
      if (!(local_ptr->cur_rep % NUM_ROTS)) { // only one first pass through
        for (v = 0; v < local_ptr->num_verts; v++) {
          local_ptr->color[v][0] = (float)nrand(255);
          local_ptr->color[v][1] = (float)nrand(255);
          local_ptr->color[v][2] = (float)nrand(255);
        }
      }
      break;
    case 3:
      // iterated RGB * texture
      // rotated textures
      // constant depth
      local_ptr->flags = DO_ITER_RGB | DO_ROTATE_TEXTURE;
      if (!(local_ptr->cur_rep % NUM_ROTS)) { // only one first pass through
        for (v = 0; v < local_ptr->num_verts; v++) {
          local_ptr->color[v][0] = (float)nrand(255);
          local_ptr->color[v][1] = (float)nrand(255);
          local_ptr->color[v][2] = (float)nrand(255);
        }
      }
      break;
    case 4:
      // decal texture
      // rotated tris
      // texture changes depth
      local_ptr->flags = DO_TWEAK_DEPTH;
      break;
    case 5:
      // decal texture
      // rotated textures
      // texture changes depth
      local_ptr->flags = DO_ROTATE_TEXTURE | DO_TWEAK_DEPTH;
      break;
    case 6:
      // iterated RGB * texture
      // rotated tris
      // texture changes depth
      local_ptr->flags = DO_ITER_RGB | DO_TWEAK_DEPTH;
      if (!(local_ptr->cur_rep % NUM_ROTS)) { // only one first pass through
        for (v = 0; v < local_ptr->num_verts; v++) {
          local_ptr->color[v][0] = (float)nrand(255);
          local_ptr->color[v][1] = (float)nrand(255);
          local_ptr->color[v][2] = (float)nrand(255);
        }
      }
      break;
    case 7:
      // iterated RGB * texture
      // rotated textures
      // texture changes depth
      local_ptr->flags = DO_ITER_RGB | DO_ROTATE_TEXTURE | DO_TWEAK_DEPTH;
      if (!(local_ptr->cur_rep % NUM_ROTS)) { // only one first pass through
        for (v = 0; v < local_ptr->num_verts; v++) {
          local_ptr->color[v][0] = (float)nrand(255);
          local_ptr->color[v][1] = (float)nrand(255);
          local_ptr->color[v][2] = (float)nrand(255);
        }
      }
      break;
    default: 
      break;
  }

  // Set the texture/RGB/depth/alpha/bilinear filtering combo for this test.
  set_mode(state, local_ptr);

  // Adjust layout on screen
  if (local_ptr->use_window_coords) {
    // If we're using window coords, scale ourselves up to the conform
    // window dimensions, and offset appropriately.
    local_ptr->scale_x  = (float)state->conform_width;
    local_ptr->scale_y  = (float)state->conform_height;
    local_ptr->offset_x = (float)(state->conform_x + (state->conform_width / 2.0f));
    local_ptr->offset_y = (float)(state->conform_y + (state->conform_height / 2.0f));
  } else {
    // If we're using clip coords, we need to scale ourselves up by
    // our W value.
    local_ptr->scale_x = 2.0f;                 // -1 to +1
    local_ptr->scale_y = 2.0f;                 // -1 to +1
    local_ptr->scale_x *= CLIP_COORDS_W_VALUE; // to compensate for W
    local_ptr->scale_y *= CLIP_COORDS_W_VALUE; // to compensate for W
    local_ptr->offset_x = 0.0f;
    local_ptr->offset_y = 0.0f;
  }

  Vertex *verts = local_ptr->verts;
  Vertex xformed_verts[MAX_VERTS];

  if (local_ptr->flags & DO_ROTATE_TEXTURE) {
    // leave the prims, rotate the textures
    rot_tex_verts_z(local_ptr, xformed_verts, verts);
  } else {
    // leave the textures, rotate the prims
    rot_verts_z(local_ptr, xformed_verts, verts);
  }

  if (local_ptr->flags & DO_TWEAK_DEPTH) {
    xformed_verts[1].tmu[0].s *= 0.2f;
    xformed_verts[1].tmu[0].t *= 0.2f;
    xformed_verts[1].tmu[0].q *= 0.2f;
  }
  
  if (local_ptr->do_aa) {
    // make it a more interesting test - don't AA one side.
    grAADrawTriangle(&xformed_verts[0], 
                     &xformed_verts[1], 
                     &xformed_verts[2],
                     FXTRUE, FXTRUE, FXFALSE);
    add_command_data(state, "grAADrawTriangle()", 6,   
                            NULL, NULL, NULL,
                            FXTRUE, FXTRUE, FXFALSE);
                             
  } else {
    grDrawTriangle(&xformed_verts[0], 
                   &xformed_verts[1], 
                   &xformed_verts[2]);
    add_command_data(state, "grDrawTriangle()", 0);  
  }
  add_vertex_data(state, xformed_verts, 3);

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
    end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);
  }

  local_ptr->cur_rep++;

  return TEST_PASS; // return value ignored

}

void help_conform(conform_state *state)
{
  fprintf(stderr,"   -aa     enable anti-aliasing\n");
  fprintf(stderr,"   -wc     use window coords instead of clip coords\n");
  fprintf(stderr,"   -bf     use bilinear filtering and mipmapping\n");
  fprintf(stderr,"   -nr <n> the number of rotations of each prim\n");
}


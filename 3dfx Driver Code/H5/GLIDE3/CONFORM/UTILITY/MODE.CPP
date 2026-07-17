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
@func fill_in_static_state
@date 8/14/98
@arg conform_state *state - current conformance state
@return none
@imp
@key
@sect
@html
Copy static hardware specific state to the conformance state structure
@end
-----------------------------------------------------------------------------*/
void fill_in_static_state(conform_state *state)
{
  if(!grGet(GR_NUM_FB, sizeof(long), &(state->num_fb))) {
    // call failed - system error
    log_message(state, "Could not read GR_NUM_FB from Glide state");
    return;
  }
  
  if(!grGet(GR_NUM_TMU, sizeof(long), &(state->num_tmu))) {
    // call failed - system error
    log_message(state, "Could not read GR_NUM_TMU from Glide state");
    return;
  }
  
  if(!grGet(GR_REVISION_FB, sizeof(long), &(state->fb_revision))) {
    // call failed - system error
    log_message(state, "Could not read GR_REVISION_FB from Glide state");
    return;
  }
  
  if(!grGet(GR_REVISION_TMU, sizeof(long), &(state->tmu_revision))) {
    // call failed - system error
    log_message(state, "Could not read GR_REVISION_TMU from Glide state");
    return;
  }
  
// DEBUG - how does this work
//  if(!grGet(GR_SUPPORTS_PASSTHRU, sizeof(long), &(state->passthru))) {
//    // call failed - system error
//    log_message(state, "Could not read GR_SUPPORTS_PASSTHRU from Glide state");
//    return;
//  }
  
  if(!grGet(GR_TEXTURE_ALIGN, sizeof(long), &(state->tex_align))) {
    // call failed - system error
    log_message(state, "Could not read GR_TEXTURE_ALIGN from Glide state");
    return;
  }


// DEBUG - this causes program faults
//  if(!grGet(GR_MAX_TEXTURE_SIZE, sizeof(long), &(state->max_tex_size))) {
//    // call failed - system error
//    log_message(state, "Could not read GR_MAX_TEXTURE SIZE from Glide state");
//    return;
//  }

// DEBUG - this causes program faults
//  if(!grGet(GR_MAX_TEXTURE_ASPECT_RATIO, sizeof(long), 
//     &(state->max_tex_aspect))) {
//    // call failed - system error
//    log_message(state, "Could not read GR_MAX_TEXTURE_ASPECT_RATIO from Glide state");
//    return;
//  }

  if(!grGet(GR_MEMORY_FB, sizeof(long), &(state->fb_mem))) {
    // call failed - system error
    log_message(state, "Could not read GR_MEMORY_FB from Glide state");
    return;
  }
  state->fb_total = state->fb_mem*state->num_fb;

  if(!grGet(GR_MEMORY_TMU, sizeof(long), &(state->tmu_mem))) {
    // call failed - system error
    log_message(state, "Could not read GR_MEMORY_TMU from Glide state");
    return;
  }
  state->tmu_total = state->tmu_mem*state->num_tmu;

  state->hardware = grGetString(GR_HARDWARE);
  state->version = grGetString(GR_VERSION);
}

/*-----------------------------------------------------------------------------
@func set_mode
@date 8/14/98
@arg conform_state *state - current conformance state
@return int - 0, if failure, -1 if success
@imp
@key
@sect
@html
Select a visual type based on the description in the conformance state
structure.  If the visual type is incompletely specified, a visual is selected
at random from the available matching visuals.  The hardware is then set to
this visual and the conformance state structure is updated to reflect the
paramters of the visual selected.
@end
-----------------------------------------------------------------------------*/
int set_mode(conform_state *state)
{
  GrResolution query;
  GrResolution *list;
  int num;
  int numbytes;
  int rnum;
  int num_trys;

  query.resolution = state->resolution;
  query.refresh = state->refresh;

  if(state->num_color_bufs == 0) {
    log_message(state,"0 color buffers specified - forcing 1");
    state->num_color_bufs = 1;
  }

  if(state->num_color_bufs == GR_QUERY_ANY) {
    query.numColorBuffers = nrand(MAX_NUM_COLOR_BUFS)+1;
  } else {
    query.numColorBuffers = state->num_color_bufs;
  }

  if(state->num_aux_bufs == GR_QUERY_ANY) {
    query.numAuxBuffers = nrand(MAX_NUM_AUX_BUFS+1);
  } else {
    query.numAuxBuffers = state->num_aux_bufs;
  }

  num_trys = 0;

  do {

    numbytes = grQueryResolutions(&query, NULL);
    num = numbytes/sizeof(GrResolution);
    list = (GrResolution *)malloc(numbytes);
    grQueryResolutions(&query, list);
    
    if(num == 0) {
      // No matching visuals
      if(++num_trys == MAX_ATTEMPTS) {
        log_message(state, "No matching visuals");
        return(0);
      } else {
        // try again
        if(state->num_color_bufs == GR_QUERY_ANY) {
          query.numColorBuffers = nrand(MAX_NUM_COLOR_BUFS)+1;
        }
        if(state->num_aux_bufs == GR_QUERY_ANY) {
          query.numAuxBuffers = nrand(MAX_NUM_AUX_BUFS+1);
        }
      }
    } else {
      rnum = nrand(num);
      state->resolution = list[rnum].resolution;
      state->refresh = list[rnum].refresh;
      state->num_color_bufs = list[rnum].numColorBuffers;
      state->num_aux_bufs = list[rnum].numAuxBuffers;
      mode_to_res(state); // fill in state->screen_width, state->screen_height
    }
    
  // make sure the tile fits on the screen
  } while ((state->conform_height > state->screen_height) &&
          (state->conform_width > state->screen_width));

  if(state->origin == GR_QUERY_ANY) {
    switch(nrand(2)) {
    case 0:
      state->origin = GR_ORIGIN_UPPER_LEFT;
      break;
    case 1:
      state->origin = GR_ORIGIN_LOWER_LEFT;
      break;
    }
  }
  
  if(state->colorformat == GR_QUERY_ANY) {
    switch(nrand(4)) {
    case 0:
      state->colorformat = GR_COLORFORMAT_RGBA;
      break;
    case 1:
      state->colorformat = GR_COLORFORMAT_ARGB;
      break;
    case 2:
      state->colorformat = GR_COLORFORMAT_BGRA;
      break;
    case 3:
      state->colorformat = GR_COLORFORMAT_ABGR;
      break;
    }
  }
  
  mode_to_res(state);
  
  if(state->conform_width == -1) {
    state->conform_width = state->screen_width;
  }
  
  if(state->conform_height == -1) {
    state->conform_height = state->screen_height;
  }

  // Create a window for the test if we haven't done so already. At some
  // point, we may run multiple tests in one window. So, if we've got a
  // window_handle set, just reuse it.

  if (!state->window_handle) {
    state->window_handle = create_window(state);
  }

  if(state->window_handle == 0) {
    log_message(state, "Could not create window");
    return(0);
  }

  if (state->use_surface_extensions) {
    int status = 0;
    state->context = surface_create_context(state);
    if (state->context) {
      status = create_surfaces(state);
      if (!status) {
        log_message(state, "Couldn't create surfaces"); 
        surface_release_context(state, state->context);
        state->context = 0;
      }
    }
  } else {

    state->context = grSstWinOpen(state->window_handle,
                                  state->resolution,
                                  state->refresh,
                                  state->colorformat,
                                  state->origin,
                                  state->num_color_bufs,
                                  state->num_aux_bufs);
  }

  if(state->context == 0) {
    log_message(state, "Could not open window");
    return(0);
  }

  free((void *)list);

  return(-1);
}

/*-----------------------------------------------------------------------------
@func mode_to_res
@date 8/14/98
@arg conform_state *state - current conformance state
@return none
@imp
@key
@sect
@html
Update the screen width and height values in the conformance state structure
based on the current resolution.
@end
-----------------------------------------------------------------------------*/
void mode_to_res(conform_state *state)
{
  switch(state->resolution) {
  case GR_RESOLUTION_320x200:
    state->screen_width = 320;
    state->screen_height = 200;
    break;
  case GR_RESOLUTION_320x240:
    state->screen_width = 320;
    state->screen_height = 240;
    break;
  case GR_RESOLUTION_400x256:
    state->screen_width = 400;
    state->screen_height = 256;
    break;
  case GR_RESOLUTION_400x300:
    state->screen_width = 400;
    state->screen_height = 300;
    break;
  case GR_RESOLUTION_512x384:
    state->screen_width = 512;
    state->screen_height = 384;
    break;
  case GR_RESOLUTION_640x200:
    state->screen_width = 640;
    state->screen_height = 200;
    break;
  case GR_RESOLUTION_640x350:
    state->screen_width = 640;
    state->screen_height = 350;
    break;
  case GR_RESOLUTION_640x400:
    state->screen_width = 640;
    state->screen_height = 400;
    break;
  case GR_RESOLUTION_640x480:
    state->screen_width = 640;
    state->screen_height = 480;
    break;
  case GR_RESOLUTION_800x600:
    state->screen_width = 800;
    state->screen_height = 600;
    break;
  case GR_RESOLUTION_960x720:
    state->screen_width = 960;
    state->screen_height = 720;
    break;
  case GR_RESOLUTION_856x480:
    state->screen_width = 856;
    state->screen_height = 480;
    break;
  case GR_RESOLUTION_512x256:
    state->screen_width = 512;
    state->screen_height = 256;
    break;
  case GR_RESOLUTION_1024x768:
    state->screen_width = 1024;
    state->screen_height = 768;
    break;
  case GR_RESOLUTION_1280x1024:
    state->screen_width = 1280;
    state->screen_height = 1024;
    break;
  case GR_RESOLUTION_1600x1200:
    state->screen_width = 1600;
    state->screen_height = 1200;
    break;
  case GR_RESOLUTION_1152x864:
    state->screen_width = 1152;
    state->screen_height = 864;
    break;
  case GR_RESOLUTION_1792x1344:
    state->screen_width = 1729;
    state->screen_height = 1344;
    break;
  case GR_RESOLUTION_1856x1392:
    state->screen_width = 1856;
    state->screen_height = 1392;
    break;
  case GR_RESOLUTION_1920x1440:
    state->screen_width = 1920;
    state->screen_height = 1440;
    break;
  case GR_RESOLUTION_2048x1536:
    state->screen_width = 2048;
    state->screen_height = 1536;
    break;
  case GR_RESOLUTION_2048x2048:
    state->screen_width = 2048;
    state->screen_height = 2048;
    break;
  default:
    log_message(state, "Undefined resolution - assuming 640x480");
    state->resolution = GR_RESOLUTION_640x480;
    state->screen_width = 640;
    state->screen_height = 480;
    break;
  }
}

/*-----------------------------------------------------------------------------
@func string_to_res
@date 8/14/98
@arg char * val
@arg conform_state *state - current conformance state
@return none
@imp
@key
@sect
@html
Translate a resolution argument string to a resolution value in the conformance
state.
@end
-----------------------------------------------------------------------------*/
void string_to_res(char *val, conform_state *state)
{
  // in case of bad arg - assume any
  state->resolution = GR_QUERY_ANY;
  if(!_stricmp(val,"320x200")) state->resolution = GR_RESOLUTION_320x200;
  if(!_stricmp(val,"320x240")) state->resolution = GR_RESOLUTION_320x240;
  if(!_stricmp(val,"400x256")) state->resolution = GR_RESOLUTION_400x256;
  if(!_stricmp(val,"400x300")) state->resolution = GR_RESOLUTION_400x300;
  if(!_stricmp(val,"512x384")) state->resolution = GR_RESOLUTION_512x384;
  if(!_stricmp(val,"640x200")) state->resolution = GR_RESOLUTION_640x200;
  if(!_stricmp(val,"640x350")) state->resolution = GR_RESOLUTION_640x350;
  if(!_stricmp(val,"640x400")) state->resolution = GR_RESOLUTION_640x400;
  if(!_stricmp(val,"640x480")) state->resolution = GR_RESOLUTION_640x480;
  if(!_stricmp(val,"800x600")) state->resolution = GR_RESOLUTION_800x600;
  if(!_stricmp(val,"960x720")) state->resolution = GR_RESOLUTION_960x720;
  if(!_stricmp(val,"856x480")) state->resolution = GR_RESOLUTION_856x480;
  if(!_stricmp(val,"512x256")) state->resolution = GR_RESOLUTION_512x256;
  if(!_stricmp(val,"1024x768")) state->resolution = GR_RESOLUTION_1024x768;
  if(!_stricmp(val,"1280x1024")) state->resolution = GR_RESOLUTION_1280x1024;
  if(!_stricmp(val,"1600x1200")) state->resolution = GR_RESOLUTION_1600x1200;
}

/*-----------------------------------------------------------------------------
@func string_to_order
@date 8/14/98
@arg char * val
@arg conform_state *state - current conformance state
@return none
@imp
@key
@sect
@html
Translate a color format argument string to a colorformat  value in the
conformance state.
@end
-----------------------------------------------------------------------------*/
void string_to_order(char *val, conform_state *state)
{
  // in case of bad arg - assume any
  state->colorformat = GR_QUERY_ANY;
  if(!_stricmp(val,"rgba")) state->colorformat = GR_COLORFORMAT_RGBA;
  if(!_stricmp(val,"argb")) state->colorformat = GR_COLORFORMAT_ARGB;
  if(!_stricmp(val,"bgra")) state->colorformat = GR_COLORFORMAT_BGRA;
  if(!_stricmp(val,"abgr")) state->colorformat = GR_COLORFORMAT_ABGR;
}

/*-----------------------------------------------------------------------------
@func print_res
@date 8/14/98
@arg conform_state *state - current conformance state
@return none
@imp
@key
@sect
@html
Print current resolution to the conformance log file
@end
-----------------------------------------------------------------------------*/
void print_res(conform_state *state)
{
  if(state->resolution == GR_RESOLUTION_320x200) {
    fprintf(state->logfile,"320x200");
    return;
  }
  if(state->resolution == GR_RESOLUTION_320x240) {
    fprintf(state->logfile,"320x240");
    return;
  }
  if(state->resolution == GR_RESOLUTION_400x256) {
    fprintf(state->logfile,"400x256");
    return;
  }
  if(state->resolution == GR_RESOLUTION_400x300) {
    fprintf(state->logfile,"400x300");
    return;
  }
  if(state->resolution == GR_RESOLUTION_512x384) {
    fprintf(state->logfile,"512x384");
    return;
  }
  if(state->resolution == GR_RESOLUTION_640x200) {
    fprintf(state->logfile,"640x200");
    return;
  }
  if(state->resolution == GR_RESOLUTION_640x350) {
    fprintf(state->logfile,"640x350");
    return;
  }
  if(state->resolution == GR_RESOLUTION_640x400) {
    fprintf(state->logfile,"640x400");
    return;
  }
  if(state->resolution == GR_RESOLUTION_640x480) {
    fprintf(state->logfile,"640x480");
    return;
  }
  if(state->resolution == GR_RESOLUTION_800x600) {
    fprintf(state->logfile,"800x600");
    return;
  }
  if(state->resolution == GR_RESOLUTION_960x720) {
    fprintf(state->logfile,"960x720");
    return;
  }
  if(state->resolution == GR_RESOLUTION_856x480) {
    fprintf(state->logfile,"856x480");
    return;
  }
  if(state->resolution == GR_RESOLUTION_512x256) {
    fprintf(state->logfile,"512x256");
    return;
  }
  if(state->resolution == GR_RESOLUTION_1024x768) {
    fprintf(state->logfile,"1024x768");
    return;
  }
  if(state->resolution == GR_RESOLUTION_1280x1024) {
     fprintf(state->logfile,"1280x1024");
     return;
   }
  if(state->resolution == GR_RESOLUTION_1600x1200) {
     fprintf(state->logfile,"1600x1200");
     return;
   }
  fprintf(state->logfile,"Unknown");
}

/*-----------------------------------------------------------------------------
@func print_ref
@date 8/14/98
@arg conform_state *state - current conformance state
@return none
@imp
@key
@sect
@html
Print current refresh rate to the conformance log file
@end
-----------------------------------------------------------------------------*/
void print_ref(conform_state *state)
{
  if(state->refresh == GR_REFRESH_60Hz) {
    fprintf(state->logfile,"60Hz");
    return;
  }
  if(state->refresh == GR_REFRESH_70Hz) {
    fprintf(state->logfile,"70Hz");
    return;
  }
  if(state->refresh == GR_REFRESH_72Hz) {
    fprintf(state->logfile,"72Hz");
    return;
  }
  if(state->refresh == GR_REFRESH_75Hz) {
    fprintf(state->logfile,"75Hz");
    return;
  }
  if(state->refresh == GR_REFRESH_80Hz) {
    fprintf(state->logfile,"80Hz");
    return;
  }
  if(state->refresh == GR_REFRESH_85Hz) {
    fprintf(state->logfile,"85Hz");
    return;
  }
  if(state->refresh == GR_REFRESH_90Hz) {
    fprintf(state->logfile,"90Hz");
    return;
  }
  if(state->refresh == GR_REFRESH_100Hz) {
    fprintf(state->logfile,"100Hz");
    return;
  }
  if(state->refresh == GR_REFRESH_120Hz) {
    fprintf(state->logfile,"120Hz");
    return;
  }
  fprintf(state->logfile,"Unknown");
}

/*-----------------------------------------------------------------------------
@func print_origin
@date 8/14/98
@arg conform_state *state - current conformance state
@return none
@imp
@key
@sect
@html
Print current screen origin to the conformance log file
@end
-----------------------------------------------------------------------------*/
void print_origin(conform_state *state)
{
  if(state->origin == GR_ORIGIN_UPPER_LEFT) {
    fprintf(state->logfile,"UPPER_LEFT");
    return;
  }
  if(state->origin == GR_ORIGIN_LOWER_LEFT) {
    fprintf(state->logfile,"LOWER_LEFT");
    return;
  }
  fprintf(state->logfile,"Unknown");
}

/*-----------------------------------------------------------------------------
@func print_format
@date 8/14/98
@arg conform_state *state - current conformance state
@return none
@imp
@key
@sect
@html
Print current colorformat to the conformance log file
@end
-----------------------------------------------------------------------------*/
void print_format(conform_state *state)
{
  if(state->colorformat == GR_COLORFORMAT_RGBA) {
    fprintf(state->logfile,"RGBA");
    return;
  }
  if(state->colorformat == GR_COLORFORMAT_ARGB) {
    fprintf(state->logfile,"ARGB");
    return;
  }
  if(state->colorformat == GR_COLORFORMAT_BGRA) {
    fprintf(state->logfile,"BGRA");
    return;
  }
  if(state->colorformat == GR_COLORFORMAT_ABGR) {
    fprintf(state->logfile,"ABGR");
    return;
  }
  fprintf(state->logfile,"Unknown");
}

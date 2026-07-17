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

typedef void GrGC;

extern "C" {
#include "fxhwc.h"
#include "gsfc.h"
#include "gdiag.h"
}

typedef void * GrSurface_t; // shouldn't this be defined somewhere?

static GdEnvironment *env; 
HwcBuffer frontBuffer, backBuffer, auxBuffer, texBuffer;

/*------------------------------------------------------------------------
@func create_texture_surface
@date 1/27/99
@arg conform_state *state - the framework's state
@arg GrChipID_t tmu - the tmu to point at this surface
@arg GrTexInfo *texInfo - description of texture we want to store. This
                          is used to figure out the dimensions of the
                          surface we need to create.
@return FXTRUE if successful, FXFALSE otherwise
@html
Called to create a texture surface for a tmu. Sets up 'tmu' 
to use the surface created. surfaces are stored in the 
state->surface_textures[tmu] array. 
@end
------------------------------------------------------------------------*/
FxBool create_texture_surface(conform_state *state, 
                              GrChipID_t tmu, GrTexInfo *texInfo)
{
  // ...
  
  return FXFALSE;
}

/*------------------------------------------------------------------------
@func create_surfaces
@date 1/10/99
@arg conform_state *state - the framework's state
@return FXTRUE if successful, FXFALSE otherwise
@html
Called by the framework (in lieu of grSstWinOpen()) to create the color,
aux, and texture buffers to be used by a test. It uses the glide3
surface extensions and direct draw surfaces.
to be called. 
@end
------------------------------------------------------------------------*/
FxBool create_surfaces(conform_state *state)
{
  GrSurfaceTexType_t tType;
  int                  status = 1;

  void (FX_CALL *grSurfaceSetRenderingSurface) ( GrSurface_t sfc );
  void (FX_CALL *grSurfaceSetAuxSurface) ( GrSurface_t sfc );
  void (FX_CALL *grSurfaceSetTextureSurface) ( GrSurface_t sfc );

   grSurfaceSetRenderingSurface = (void (FX_CALL *)(GrSurface_t sfc))
    (void*)grGetProcAddress( "grSurfaceSetRenderingSurfaceExt" );
  grSurfaceSetAuxSurface = (void (FX_CALL *)(GrSurface_t sfc))
    (void*)grGetProcAddress( "grSurfaceSetAuxSurfaceExt" );
  grSurfaceSetTextureSurface = (void (FX_CALL *)(GrSurface_t sfc))
    (void*)grGetProcAddress( "grSurfaceSetTextureSurfaceExt" );

  // make sure we got 'em
  if ( !grSurfaceSetRenderingSurface ) {
    log_message(state, "Couldn't get grSurfaceSetRenderingSurface proc");
    return FXFALSE;
  }
  if ( !grSurfaceSetAuxSurface ) {
    log_message(state, "Couldn't get grSurfaceSetAuxSurface proc");
    return FXFALSE;
  }
  if ( !grSurfaceSetAuxSurface ) {
    log_message(state, "Couldn't get grSurfaceSetTextureSurface proc");
    return FXFALSE;
  }

  // TODO clean things up when env stabalizes
  env = gdGetEnv(); 
  env->width = state->conform_width;
  env->height = state->conform_height;
  tType = env->texAGP ? GR_SURFACETEXTYPE_AGP : GR_SURFACETEXTYPE_FB; 

  if (!grSurfaceInitializeSurfaces(state->window_handle, HWC_RESOLUTION_NONE, GR_REFRESH_60Hz, tType, env->tmuRamMB*1024, 
                                   ~0UL, GR_COLORFORMAT_ARGB, state->origin, 2, state->num_aux_bufs))
    return FXFALSE;

  gdGetBuffer(&frontBuffer, HWC_BUF_3D_FRONT); 
  gdGetBuffer(&backBuffer,  HWC_BUF_3D_BACK); 
  gdGetBuffer(&auxBuffer,   HWC_BUF_3D_AUX); 
  //gdGetBuffer(&texBuffer,   HWC_BUF_TEXTURE); 

  state->surface_front   = (void *)&frontBuffer; 
  state->surface_back    = (void *)&backBuffer; 
  state->surface_aux     = (void *)&auxBuffer; 
  //state->surface_texture = (void *)&texBuffer; 

  return FXTRUE;
}

/*------------------------------------------------------------------------
@func release_surfaces
@date 1/10/99
@arg conform_state *state - the framework's state
@html
Called by the framework to release resources allocated by
create_surfaces()
@end
------------------------------------------------------------------------*/
void release_surfaces(conform_state *state)
{
  // TODO free buffers
}

/*--------------------------------------------------------------------------
@func get_surface_bits 
@date 1/12/99
@arg conform_state *state - the conformance framework state
@arg void* surface - the surface whose bits to get
@arg FxU32 format - the format (currently ignored)
@arg GrOriginLocation_t origin - the origin to use
@arg void* bit_buf - the destination buffer
@return FXFALSE
@imp
@key
@sect
@html
Retrieve the complete contents of a surface.
@end
--------------------------------------------------------------------------*/
FxBool get_surface_bits(conform_state *state, void *src, 
                        FxU32 format, GrOriginLocation_t origin,
                        void *bit_buf)
{
 return FXFALSE;
}


/*--------------------------------------------------------------------------
@func get_surface_pixel 
@date 1/12/99
@arg conform_state *state - the conformance framework state
@arg void* surface - the surface from which to read the pixel
@arg FxU32 format - the format (currently ignored)
@arg GrOriginLocation_t origin - the origin to use
@arg int x - the X location of the pixel
@arg int y - the Y location of the pixel
@arg FxU32 *pixel - the location at which to return the pixel
@arg 
@return  FXTRUE if succesful, FXFALSE otherwise
@imp
@key
@sect
@html
Retrieve a single pixel from a surface.
@end
--------------------------------------------------------------------------*/
#define GetRValue(_pix) ((_pix >> 16 ) & 0xff)
#define GetGValue(_pix) ((_pix >>  8 ) & 0xff)
#define GetBValue(_pix) (_pix          & 0xff)
FxBool get_surface_pixel(conform_state *state, void *surface, 
                         FxU32 format, GrOriginLocation_t origin,
                         int x, int y, FxU32 *pixout)
{
  FxBool status = FXTRUE;
  FxU32 pixin;

  pixin = gdReadColor(HWC_BUF_3D_FRONT, x, y);

  switch (format) { 
    case GR_COLORFORMAT_RGBA:
     *pixout = (GetRValue(pixin)<<24) | (GetGValue(pixin)<<16) | 
               (GetBValue(pixin) << 8);
    break;
    case GR_COLORFORMAT_BGRA:
     *pixout = (GetBValue(pixin)<<24) | (GetGValue(pixin)<<16) | 
               (GetRValue(pixin) << 8);
    break;
    case GR_COLORFORMAT_ABGR:
     *pixout = 
       (GetBValue(pixin)<<16) | (GetGValue(pixin)<<8) | GetRValue(pixin);
    break;
    default:
    case GR_COLORFORMAT_ARGB:
     *pixout = 
       (GetRValue(pixin)<<16) | (GetGValue(pixin)<<8) | GetBValue(pixin);
    break;
  }

  return status;
}


/*--------------------------------------------------------------------------
@func blt_surface 
@date 1/12/99
@arg conform_state *state - the conformance framework state
@arg void* dst - the destination surface 
@arg void* src - the source surface
@return  FXTRUE if succesful, FXFALSE otherwise
@imp
@key
@sect
@html
Copy one surface onto another. This assumes they're the same size.
@end
--------------------------------------------------------------------------*/

FxBool blt_surface(conform_state *state, void *dst, void *src)
{
  GrGC                *gc = (GrGC *)state->context;

  grSurfaceBlit(src, dst, 0, 0, 0, 0, frontBuffer.width, frontBuffer.height , 1.0f, 1.0f);

  return FXTRUE;
}

/*--------------------------------------------------------------------------
@func lock_surface 
@date 1/29/99
@arg conform_state *state - the conformance framework state
@arg void* surface - the surface to lock 
@return  void* - a ptr to the pixels in the surface, or NULL if
         unsuccessful.
@imp
@key
@sect
@html
Lock a surface for direct access, and return a pointer to it's pixels.
@end
--------------------------------------------------------------------------*/
void* lock_surface(conform_state *state, void *surface)
{
   // ...

   return NULL;
}

/*--------------------------------------------------------------------------
@func unlock_surface 
@date 1/29/99
@arg conform_state *state - the conformance framework state
@arg void* surface - the surface to unlock 
@arg void* ptr     - a pointer to the surfaces pixels, previously
                     retrieved by a call to lock_surface()
@return 
@imp
@key
@sect
@html
Unlock a surface previously locked by lock_surface()
@end
--------------------------------------------------------------------------*/
void unlock_surface(conform_state *state, void *surface, void *ptr)
{
   if (!ptr) {
     return;
   }

   // ...
}

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

/**
 **  This file contains the stubs of platform-dependent surface code, 
 **  and is linked in when we don't have a specific implementation.
 **/

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
  return FXFALSE;
}

/*--------------------------------------------------------------------------
@func create_surfaces 
@date 1/12/99
@arg conform_state *state - the conformance framework state
@return FXFALSE 
@imp
@key
@sect
@html
Create all surfaces necessary for this test. 
@end
--------------------------------------------------------------------------*/
FxBool create_surfaces(conform_state *state)
{
  log_message(state, "create_surfaces() STUB");
  return FXFALSE;
}

/*--------------------------------------------------------------------------
@func release_surfaces 
@date 1/12/99
@arg conform_state *state - the conformance framework state
@return 
@imp
@key
@sect
@html
Release surfaces created by create_surface()
@end
--------------------------------------------------------------------------*/
void release_surfaces(conform_state *state)
{
  log_message(state, "release_surfaces() STUB");
}

/*--------------------------------------------------------------------------
@func get_surface_bits 
@date 1/12/99
@arg conform_state *state - the conformance framework state
@arg void* surface - the surface whose bits to get
@arg FxU32 format - the format (currently ignored)
@arg GrOriginLocation_t origin - the origin to use
@arg void* bit_buf - the destination buffer
@return  FXFALSE
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
  log_message(state, "get_surface_bits() STUB");
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
@return FXFALSE
@imp
@key
@sect
@html
Retrieve a single pixel from a surface.
@end
--------------------------------------------------------------------------*/
FxBool get_surface_pixel(conform_state *state, void *src, 
                         FxU32 format, GrOriginLocation_t origin,
                         int x, int y, FxU32 *pixel)
{
  log_message(state, "get_surface_pixel() STUB");
  return FXFALSE;
}


/*--------------------------------------------------------------------------
@func blt_surface 
@date 1/12/99
@arg conform_state *state - the conformance framework state
@arg void* dst - the destination surface 
@arg void* src - the source surface
@return  FXFALSE
@imp
@key
@sect
@html
Copy one surface onto another. This assumes they're the same size.
@end
--------------------------------------------------------------------------*/
FxBool blt_surface(conform_state *state, void *dst, void *src)
{
  return FXFALSE;
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
}


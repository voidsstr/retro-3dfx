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

#ifndef __WIN32__
# error "This file should only be compiled on WIN32"
#endif // NOT __WIN32__

#ifndef DIRECTDRAW_AVAILABLE
# error "This file requires the Direct Draw SDK to be available"
#endif // NOT DIRECTDRAW_AVAILABLE 

#include "conform.h"
#include <windows.h>

#include <ddraw.h>
#include <d3d.h>

// A convenience wrapper around the DirectDraw routines that reports 
// if they fail.
#define DD_TRY(_state, _status, _X )                       \
{                                                          \
  HRESULT hr = (_X);                                       \
  if ( hr != DD_OK ) {                                     \
    char *_errBuf = (char *)calloc(1024, 1);               \
    sprintf(_errBuf, "DirectDraw API Error: %s @ line %d", \
                     ddErrorToString(hr), __LINE__);       \
    log_message(_state, _errBuf);                          \
    fprintf(stderr, "%s\n", _errBuf);                      \
    free(_errBuf);                                         \
    _status = FXFALSE;                                     \
  }                                                        \
}

typedef void * GrSurface_t; // shouldn't this be defined somewhere?


// stolen verbatim from gsurf/main.c
static 
char* ddErrorToString(HRESULT error)
{
    switch(error) {
        case DD_OK:
            /* Also includes D3D_OK and D3DRM_OK */
            return "No error.\0";
        case DDERR_ALREADYINITIALIZED:
            return "This object is already initialized.\0";
        case DDERR_BLTFASTCANTCLIP:
            return "Return if a clipper object is attached to the source surface passed into a BltFast call.\0";
        case DDERR_CANNOTATTACHSURFACE:
            return "This surface can not be attached to the requested surface.\0";
        case DDERR_CANNOTDETACHSURFACE:
            return "This surface can not be detached from the requested surface.\0";
        case DDERR_CANTCREATEDC:
            return "Windows can not create any more DCs.\0";
        case DDERR_CANTDUPLICATE:
            return "Can't duplicate primary & 3D surfaces, or surfaces that are implicitly created.\0";
        case DDERR_CLIPPERISUSINGHWND:
            return "An attempt was made to set a cliplist for a clipper object that is already monitoring an hwnd.\0";
        case DDERR_COLORKEYNOTSET:
            return "No src color key specified for this operation.\0";
        case DDERR_CURRENTLYNOTAVAIL:
            return "Support is currently not available.\0";
        case DDERR_DIRECTDRAWALREADYCREATED:
            return "A DirectDraw object representing this driver has already been created for this process.\0";
        case DDERR_EXCEPTION:
            return "An exception was encountered while performing the requested operation.\0";
        case DDERR_EXCLUSIVEMODEALREADYSET:
            return "An attempt was made to set the cooperative level when it was already set to exclusive.\0";
        case DDERR_GENERIC:
            return "Generic failure.\0";
        case DDERR_HEIGHTALIGN:
            return "Height of rectangle provided is not a multiple of reqd alignment.\0";
        case DDERR_HWNDALREADYSET:
            return "The CooperativeLevel HWND has already been set. It can not be reset while the process has surfaces or palettes created.\0";
        case DDERR_HWNDSUBCLASSED:
            return "HWND used by DirectDraw CooperativeLevel has been subclassed, this prevents DirectDraw from restoring state.\0";
        case DDERR_IMPLICITLYCREATED:
            return "This surface can not be restored because it is an implicitly created surface.\0";
        case DDERR_INCOMPATIBLEPRIMARY:
            return "Unable to match primary surface creation request with existing primary surface.\0";
        case DDERR_INVALIDCAPS:
            return "One or more of the caps bits passed to the callback are incorrect.\0";
        case DDERR_INVALIDCLIPLIST:
            return "DirectDraw does not support the provided cliplist.\0";
        case DDERR_INVALIDDIRECTDRAWGUID:
            return "The GUID passed to DirectDrawCreate is not a valid DirectDraw driver identifier.\0";
        case DDERR_INVALIDMODE:
            return "DirectDraw does not support the requested mode.\0";
        case DDERR_INVALIDOBJECT:
            return "DirectDraw received a pointer that was an invalid DIRECTDRAW object.\0";
        case DDERR_INVALIDPARAMS:
            return "One or more of the parameters passed to the function are incorrect.\0";
        case DDERR_INVALIDPIXELFORMAT:
            return "The pixel format was invalid as specified.\0";
        case DDERR_INVALIDPOSITION:
            return "Returned when the position of the overlay on the destination is no longer legal for that destination.\0";
        case DDERR_INVALIDRECT:
            return "Rectangle provided was invalid.\0";
        case DDERR_LOCKEDSURFACES:
            return "Operation could not be carried out because one or more surfaces are locked.\0";
        case DDERR_NO3D:
            return "There is no 3D present.\0";
        case DDERR_NOALPHAHW:
            return "Operation could not be carried out because there is no alpha accleration hardware present or available.\0";
        case DDERR_NOBLTHW:
            return "No blitter hardware present.\0";
        case DDERR_NOCLIPLIST:
            return "No cliplist available.\0";
        case DDERR_NOCLIPPERATTACHED:
            return "No clipper object attached to surface object.\0";
        case DDERR_NOCOLORCONVHW:
            return "Operation could not be carried out because there is no color conversion hardware present or available.\0";
        case DDERR_NOCOLORKEY:
            return "Surface doesn't currently have a color key\0";
        case DDERR_NOCOLORKEYHW:
            return "Operation could not be carried out because there is no hardware support of the destination color key.\0";
        case DDERR_NOCOOPERATIVELEVELSET:
            return "Create function called without DirectDraw object method SetCooperativeLevel being called.\0";
        case DDERR_NODC:
            return "No DC was ever created for this surface.\0";
        case DDERR_NODDROPSHW:
            return "No DirectDraw ROP hardware.\0";
        case DDERR_NODIRECTDRAWHW:
            return "A hardware-only DirectDraw object creation was attempted but the driver did not support any hardware.\0";
        case DDERR_NOEMULATION:
            return "Software emulation not available.\0";
        case DDERR_NOEXCLUSIVEMODE:
            return "Operation requires the application to have exclusive mode but the application does not have exclusive mode.\0";
        case DDERR_NOFLIPHW:
            return "Flipping visible surfaces is not supported.\0";
        case DDERR_NOGDI:
            return "There is no GDI present.\0";
        case DDERR_NOHWND:
            return "Clipper notification requires an HWND or no HWND has previously been set as the CooperativeLevel HWND.\0";
        case DDERR_NOMIRRORHW:
            return "Operation could not be carried out because there is no hardware present or available.\0";
        case DDERR_NOOVERLAYDEST:
            return "Returned when GetOverlayPosition is called on an overlay that UpdateOverlay has never been called on to establish a destination.\0";
        case DDERR_NOOVERLAYHW:
            return "Operation could not be carried out because there is no overlay hardware present or available.\0";
        case DDERR_NOPALETTEATTACHED:
            return "No palette object attached to this surface.\0";
        case DDERR_NOPALETTEHW:
            return "No hardware support for 16 or 256 color palettes.\0";
        case DDERR_NORASTEROPHW:
            return "Operation could not be carried out because there is no appropriate raster op hardware present or available.\0";
        case DDERR_NOROTATIONHW:
            return "Operation could not be carried out because there is no rotation hardware present or available.\0";
        case DDERR_NOSTRETCHHW:
            return "Operation could not be carried out because there is no hardware support for stretching.\0";
        case DDERR_NOT4BITCOLOR:
            return "DirectDrawSurface is not in 4 bit color palette and the requested operation requires 4 bit color palette.\0";
        case DDERR_NOT4BITCOLORINDEX:
            return "DirectDrawSurface is not in 4 bit color index palette and the requested operation requires 4 bit color index palette.\0";
        case DDERR_NOT8BITCOLOR:
            return "DirectDrawSurface is not in 8 bit color mode and the requested operation requires 8 bit color.\0";
        case DDERR_NOTAOVERLAYSURFACE:
            return "Returned when an overlay member is called for a non-overlay surface.\0";
        case DDERR_NOTEXTUREHW:
            return "Operation could not be carried out because there is no texture mapping hardware present or available.\0";
        case DDERR_NOTFLIPPABLE:
            return "An attempt has been made to flip a surface that is not flippable.\0";
        case DDERR_NOTFOUND:
            return "Requested item was not found.\0";
        case DDERR_NOTLOCKED:
            return "Surface was not locked.  An attempt to unlock a surface that was not locked at all, or by this process, has been attempted.\0";
        case DDERR_NOTPALETTIZED:
            return "The surface being used is not a palette-based surface.\0";
        case DDERR_NOVSYNCHW:
            return "Operation could not be carried out because there is no hardware support for vertical blank synchronized operations.\0";
        case DDERR_NOZBUFFERHW:
            return "Operation could not be carried out because there is no hardware support for zbuffer blitting.\0";
        case DDERR_NOZOVERLAYHW:
            return "Overlay surfaces could not be z layered based on their BltOrder because the hardware does not support z layering of overlays.\0";
        case DDERR_OUTOFCAPS:
            return "The hardware needed for the requested operation has already been allocated.\0";
        case DDERR_OUTOFMEMORY:
            return "DirectDraw does not have enough memory to perform the operation.\0";
        case DDERR_OUTOFVIDEOMEMORY:
            return "DirectDraw does not have enough memory to perform the operation.\0";
        case DDERR_OVERLAYCANTCLIP:
            return "The hardware does not support clipped overlays.\0";
        case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
            return "Can only have ony color key active at one time for overlays.\0";
        case DDERR_OVERLAYNOTVISIBLE:
            return "Returned when GetOverlayPosition is called on a hidden overlay.\0";
        case DDERR_PALETTEBUSY:
            return "Access to this palette is being refused because the palette is already locked by another thread.\0";
        case DDERR_PRIMARYSURFACEALREADYEXISTS:
            return "This process already has created a primary surface.\0";
        case DDERR_REGIONTOOSMALL:
            return "Region passed to Clipper::GetClipList is too small.\0";
        case DDERR_SURFACEALREADYATTACHED:
            return "This surface is already attached to the surface it is being attached to.\0";
        case DDERR_SURFACEALREADYDEPENDENT:
            return "This surface is already a dependency of the surface it is being made a dependency of.\0";
        case DDERR_SURFACEBUSY:
            return "Access to this surface is being refused because the surface is already locked by another thread.\0";
        case DDERR_SURFACEISOBSCURED:
            return "Access to surface refused because the surface is obscured.\0";
        case DDERR_SURFACELOST:
            return "Access to this surface is being refused because the surface memory is gone. The DirectDrawSurface object representing this surface should have Restore called on it.\0";
        case DDERR_SURFACENOTATTACHED:
            return "The requested surface is not attached.\0";
        case DDERR_TOOBIGHEIGHT:
            return "Height requested by DirectDraw is too large.\0";
        case DDERR_TOOBIGSIZE:
            return "Size requested by DirectDraw is too large, but the individual height and width are OK.\0";
        case DDERR_TOOBIGWIDTH:
            return "Width requested by DirectDraw is too large.\0";
        case DDERR_UNSUPPORTED:
            return "Action not supported.\0";
        case DDERR_UNSUPPORTEDFORMAT:
            return "FOURCC format requested is unsupported by DirectDraw.\0";
        case DDERR_UNSUPPORTEDMASK:
            return "Bitmask in the pixel format requested is unsupported by DirectDraw.\0";
        case DDERR_VERTICALBLANKINPROGRESS:
            return "Vertical blank is in progress.\0";
        case DDERR_WASSTILLDRAWING:
            return "Informs DirectDraw that the previous Blt which is transfering information to or from this Surface is incomplete.\0";
        case DDERR_WRONGMODE:
            return "This surface can not be restored because it was created in a different mode.\0";
        case DDERR_XALIGN:
            return "Rectangle provided was not horizontally aligned on required boundary.\0";
        case D3DERR_BADMAJORVERSION:
            return "D3DERR_BADMAJORVERSION\0";
        case D3DERR_BADMINORVERSION:
            return "D3DERR_BADMINORVERSION\0";
        case D3DERR_EXECUTE_LOCKED:
            return "D3DERR_EXECUTE_LOCKED\0";
        case D3DERR_EXECUTE_NOT_LOCKED:
            return "D3DERR_EXECUTE_NOT_LOCKED\0";
        case D3DERR_EXECUTE_CREATE_FAILED:
            return "D3DERR_EXECUTE_CREATE_FAILED\0";
        case D3DERR_EXECUTE_DESTROY_FAILED:
            return "D3DERR_EXECUTE_DESTROY_FAILED\0";
        case D3DERR_EXECUTE_LOCK_FAILED:
            return "D3DERR_EXECUTE_LOCK_FAILED\0";
        case D3DERR_EXECUTE_UNLOCK_FAILED:
            return "D3DERR_EXECUTE_UNLOCK_FAILED\0";
        case D3DERR_EXECUTE_FAILED:
            return "D3DERR_EXECUTE_FAILED\0";
        case D3DERR_EXECUTE_CLIPPED_FAILED:
            return "D3DERR_EXECUTE_CLIPPED_FAILED\0";
        case D3DERR_TEXTURE_NO_SUPPORT:
            return "D3DERR_TEXTURE_NO_SUPPORT\0";
        case D3DERR_TEXTURE_NOT_LOCKED:
            return "D3DERR_TEXTURE_NOT_LOCKED\0";
        case D3DERR_TEXTURE_LOCKED:
            return "D3DERR_TEXTURELOCKED\0";
        case D3DERR_TEXTURE_CREATE_FAILED:
            return "D3DERR_TEXTURE_CREATE_FAILED\0";
        case D3DERR_TEXTURE_DESTROY_FAILED:
            return "D3DERR_TEXTURE_DESTROY_FAILED\0";
        case D3DERR_TEXTURE_LOCK_FAILED:
            return "D3DERR_TEXTURE_LOCK_FAILED\0";
        case D3DERR_TEXTURE_UNLOCK_FAILED:
            return "D3DERR_TEXTURE_UNLOCK_FAILED\0";
        case D3DERR_TEXTURE_LOAD_FAILED:
            return "D3DERR_TEXTURE_LOAD_FAILED\0";
        case D3DERR_MATRIX_CREATE_FAILED:
            return "D3DERR_MATRIX_CREATE_FAILED\0";
        case D3DERR_MATRIX_DESTROY_FAILED:
            return "D3DERR_MATRIX_DESTROY_FAILED\0";
        case D3DERR_MATRIX_SETDATA_FAILED:
            return "D3DERR_MATRIX_SETDATA_FAILED\0";
        case D3DERR_SETVIEWPORTDATA_FAILED:
            return "D3DERR_SETVIEWPORTDATA_FAILED\0";
        case D3DERR_MATERIAL_CREATE_FAILED:
            return "D3DERR_MATERIAL_CREATE_FAILED\0";
        case D3DERR_MATERIAL_DESTROY_FAILED:
            return "D3DERR_MATERIAL_DESTROY_FAILED\0";
        case D3DERR_MATERIAL_SETDATA_FAILED:
            return "D3DERR_MATERIAL_SETDATA_FAILED\0";
        case D3DERR_LIGHT_SET_FAILED:
            return "D3DERR_LIGHT_SET_FAILED\0";
        case D3DERR_LIGHTHASVIEWPORT: 
            return "D3DERR_LIGHTHASVIEPORT\0";
        case D3DERR_LIGHTNOTINTHISVIEWPORT:
            return "D3DERR_LIGHTNOTINVIEWPORT\0";
        case D3DERR_SCENE_IN_SCENE:
            return "D3DERR_SCENE_IN_SCENE\0";
        case D3DERR_SCENE_NOT_IN_SCENE:
            return "D3DERR_SCENE_NOT_IN_SCENE\0";
        case D3DERR_SCENE_BEGIN_FAILED:
            return "D3DERR_SCENE_BEGIN_FAILED\0";
        case D3DERR_SCENE_END_FAILED:
            return "D3DERR_SCENE_END_FAILED\0";
        case D3DERR_INBEGIN:
            return "D3DERR_SCENE_IN_BEGIN\0";
        case D3DERR_NOTINBEGIN:
            return "D3DERR_SCENE_NOT_IN_BEGIN\0";
        case D3DERR_NOVIEWPORTS:
            return "D3DERR_SCENE_NO_VIEWPORTS\0";
        case D3DERR_VIEWPORTDATANOTSET:
            return "D3DERR_VIEWPORTDATANOTSET:\0";
        case D3DERR_VIEWPORTHASNODEVICE:
            return "D3DERR_VIEWPORTHASNODEVICE:\0";
//        case D3DERR_NOCURRENTVIEWPORT:
//            return "D3DERR_NOCURRENTVIEWPORT:\0";
        default:
            return "Unrecognized error value.\0";
    }
}

/*-----------------------------------------------------------------------
@func getClientRect
@html
Stolen verbatim from gsurf/main.c
@end
------------------------------------------------------------------------*/
static void 
getClientRect( HWND hwnd, RECT *rect ) 
{
  POINT pt = { 0, 0 };
  ClientToScreen( hwnd, &pt );
  GetClientRect( hwnd, rect );
  OffsetRect( rect, pt.x, pt.y );
  return;
} /* getClientRect */


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
  LPDIRECTDRAW   pDDx;
  LPDIRECTDRAW2  pDD2;
  LPDIRECTDRAWSURFACE  pTexture = NULL;
  DDSURFACEDESC desc;
  FxBool status = FXTRUE;

  // Texture extensions
  void (__stdcall *grSurfaceSetTextureSurface)(GrChipID_t tmu, 
                                               GrSurface_t sfc);
  FxBool (__stdcall *grSurfaceCalcTextureWHD)(const GrTexInfo *tInfo,
                                              FxU32 *w, FxU32 *h, FxU32 *d);

  // Try to load 'em up.
  grSurfaceSetTextureSurface = 
    (void (__stdcall *)(GrChipID_t tmu, GrSurface_t sfc))
      (void*)grGetProcAddress( "grSurfaceSetTextureSurfaceExt" );

  grSurfaceCalcTextureWHD = 
    (FxBool (__stdcall *)
      (const GrTexInfo *tInfo, FxU32 *w, FxU32 *h, FxU32 *d))
        (void*)grGetProcAddress("grSurfaceCalcTextureWHDExt");

  // Make sure we got 'em.
  if ( !grSurfaceSetTextureSurface ) {
    log_message(state, "Couldn't get grSurfaceSetTextureSurface proc");
    return 0;
  }
  if ( !grSurfaceCalcTextureWHD ) {
    log_message(state, "Couldn't get grSurfaceSetTextureSurface proc");
    return 0;
  }

  // Get DirectDraw interface
  DD_TRY(state, status, 
         DirectDrawCreate( 0, &pDDx, NULL ));
  DD_TRY(state, status, 
         pDDx->QueryInterface(IID_IDirectDraw2, (LPVOID*)&pDD2));
  pDDx->Release();

  // set up display mode 
  DD_TRY(state, status, 
         pDD2->SetCooperativeLevel((HWND)state->window_handle, DDSCL_NORMAL));

  // Initialize the framework's idea of this surface to NULL;
  state->surface_textures[tmu] = NULL;

  memset(&desc, 0, sizeof(desc));
  if (!grSurfaceCalcTextureWHD(texInfo,
                               &desc.dwWidth, &desc.dwHeight,
                               &desc.ddpfPixelFormat.dwRGBBitCount)) {
    log_message(state, 
          "grSurfaceCalcTextureWHD failed to return texture information.");
    return FXFALSE;
  }

  desc.dwSize         = sizeof( DDSURFACEDESC );
  desc.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
  desc.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_OFFSCREENPLAIN;

  // Add the pixel format stuff
  desc.dwFlags        |= DDSD_PIXELFORMAT;
  desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
  desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
  desc.ddpfPixelFormat.dwRGBBitCount *= 0x08UL;
  desc.ddpfPixelFormat.dwRBitMask = 0x0000f800UL;
  desc.ddpfPixelFormat.dwGBitMask = 0x000007e0UL;
  desc.ddpfPixelFormat.dwBBitMask = 0x0000001fUL;

  if (desc.ddpfPixelFormat.dwRGBBitCount > 16UL) {
    log_message(state, 
                "grSurfaceCalcTextureWHD returned unsupported texture bpt.");
    return FXFALSE;
  }

  if (desc.ddpfPixelFormat.dwRGBBitCount == 16UL) {
    desc.ddpfPixelFormat.dwRBitMask = 0x0000F800UL; 
    desc.ddpfPixelFormat.dwGBitMask = 0x000007E0UL;
    desc.ddpfPixelFormat.dwBBitMask = 0x0000001FUL;
  } else {
    desc.ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED8;
    desc.ddpfPixelFormat.dwRBitMask = 0x00UL; 
    desc.ddpfPixelFormat.dwGBitMask = 0x00UL;
    desc.ddpfPixelFormat.dwBBitMask = 0x00UL;
  }

  DD_TRY(state, status,
         pDD2->CreateSurface(&desc, &pTexture, NULL));

  if (status) {  
    grSurfaceSetTextureSurface(tmu, (void*)pTexture ); 
    state->surface_textures[tmu] = (void*)pTexture;
  }

  return status;
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
  LPDIRECTDRAW   pDDx;
  LPDIRECTDRAW2  pDD2;
  DDSURFACEDESC  desc;
  LPDIRECTDRAWSURFACE  pFront;
  LPDIRECTDRAWSURFACE  pBack;
  LPDIRECTDRAWSURFACE  pAux;
  LPDIRECTDRAWCLIPPER  pClipper;
  RECT                 clientRect;
  RECT                 backRect; 
  FxBool               status = FXTRUE;

  void (__stdcall *grSurfaceSetRenderingSurface) ( GrSurface_t sfc );
  void (__stdcall *grSurfaceSetAuxSurface) ( GrSurface_t sfc );

  grSurfaceSetRenderingSurface = (void (__stdcall *)(GrSurface_t sfc))
    (void*)grGetProcAddress( "grSurfaceSetRenderingSurfaceExt" );
  grSurfaceSetAuxSurface = (void (__stdcall *)(GrSurface_t sfc))
    (void*)grGetProcAddress( "grSurfaceSetAuxSurfaceExt" );

  // make sure we got 'em
  if ( !grSurfaceSetRenderingSurface ) {
    log_message(state, "Couldn't get grSurfaceSetRenderingSurface proc");
    return FXFALSE;
  }
  if ( !grSurfaceSetAuxSurface ) {
    log_message(state, "Couldn't get grSurfaceSetAuxSurface proc");
    return FXFALSE;
  }

  DD_TRY(state, status, 
         DirectDrawCreate( 0, &pDDx, NULL ));
  DD_TRY(state, status, 
         pDDx->QueryInterface(IID_IDirectDraw2, (LPVOID*)&pDD2));

  pDDx->Release();

  /* set up display mode */
  DD_TRY(state, status, 
         pDD2->SetCooperativeLevel((HWND)state->window_handle, DDSCL_NORMAL));

  /* create front buffer surface */
  memset(&desc, 0, sizeof(desc));
  desc.dwSize         = sizeof( DDSURFACEDESC );
  desc.dwFlags        = DDSD_CAPS;
  desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

  DD_TRY(state, status, 
         pDD2->CreateSurface(&desc, &pFront, NULL));

  /* create windows clipper */
  DD_TRY(state, status, 
         pDD2->CreateClipper(0, &pClipper, 0));
  DD_TRY(state, status, 
         pClipper->SetHWnd(0, (HWND)state->window_handle));
  DD_TRY(state, status, 
         pFront->SetClipper(pClipper));

  // create back buffer surface
  getClientRect((HWND)state->window_handle, &clientRect );
  backRect = clientRect;
  OffsetRect(&backRect, -backRect.left, -backRect.top );

  desc.dwSize         = sizeof( DDSURFACEDESC );
  desc.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
  desc.dwWidth        = backRect.right;
  desc.dwHeight       = backRect.bottom;
  desc.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE;

  // make it a 565 surface for the moment
  desc.dwFlags        |= DDSD_PIXELFORMAT;
  desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
  desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
  desc.ddpfPixelFormat.dwRGBBitCount  = 16;
  desc.ddpfPixelFormat.dwRBitMask = 0x0000f800UL;
  desc.ddpfPixelFormat.dwGBitMask = 0x000007e0UL;
  desc.ddpfPixelFormat.dwBBitMask = 0x0000001fUL;

  DD_TRY(state, status,
         pDD2->CreateSurface(&desc, &pBack, NULL));

  grSurfaceSetRenderingSurface( (void*)pBack );

  if (state->num_aux_bufs == 1) {
    // create depth buffer surface 
    desc.dwSize         = sizeof( DDSURFACEDESC );
    desc.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    desc.dwWidth        = backRect.right;
    desc.dwHeight       = backRect.bottom;
    desc.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE;

    // make it a 16 bit surface for the moment 
    desc.dwFlags        |= DDSD_PIXELFORMAT;
    desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
    desc.ddpfPixelFormat.dwRGBBitCount  = 16;
    desc.ddpfPixelFormat.dwRBitMask = 0x0000f800UL;
    desc.ddpfPixelFormat.dwGBitMask = 0x000007e0UL;
    desc.ddpfPixelFormat.dwBBitMask = 0x0000001fUL;

    DD_TRY(state, status,
           pDD2->CreateSurface(&desc, &pAux, NULL));

    grSurfaceSetAuxSurface( (void*)pAux );
  }

  state->surface_front   = (void *) pFront;
  state->surface_back    = (void *) pBack;
  state->surface_aux     = (void *) pAux;

  pDD2->Release();  // Does this need doing?

  return (status);
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
  if (state->surface_front) {
    ((LPDIRECTDRAWSURFACE)state->surface_front)->Release();
  }
  if (state->surface_back) {
    ((LPDIRECTDRAWSURFACE)state->surface_back)->Release();
  }
  if (state->surface_aux) {
    ((LPDIRECTDRAWSURFACE)state->surface_aux)->Release();
  }
  for (int i = 0; i < CONFORM_MAX_TMUS; i++) {
    if (state->surface_textures[i]) {
      ((LPDIRECTDRAWSURFACE)state->surface_textures[i])->Release();
    }
  }
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
Retrieve a single pixel from a surface. Uses GDI. Not fast.
@end
--------------------------------------------------------------------------*/
FxBool get_surface_pixel(conform_state *state, void *surface, 
                         FxU32 format, GrOriginLocation_t origin,
                         int x, int y, FxU32 *pixout)
{

  HDC hdc;
  FxBool status = FXTRUE;
  LPDIRECTDRAWSURFACE pSrc = (LPDIRECTDRAWSURFACE) surface;
  POINT p;
  int caps;

  // Offset the point to reflect the position of the drawing
  // surface.
  p.x = x;
  p.y = y;
  ClientToScreen((HWND)state->window_handle, &p);

  if (pSrc->GetDC(&hdc) == DD_OK) {
    // make sure the surface supports GetPixel()
    caps = GetDeviceCaps(hdc, RASTERCAPS);
    if (caps & RC_BITBLT) {
      COLORREF pixin = GetPixel(hdc, p.x, p.y); 
      // SetPixel(hdc, p.x, p.y, 0x00FFFFFF); // debugging
      if (pixin == CLR_INVALID) {
        status = FXFALSE;
      } else {
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
      }
    } else {
      log_message(state, "get_surface_pixel(): GetPixel() not supported by surface");
      status = FXFALSE;
    }
    pSrc->ReleaseDC(hdc);
  } else {
    status = FXFALSE;
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

  LPDIRECTDRAWSURFACE pDst = (LPDIRECTDRAWSURFACE) dst;
  LPDIRECTDRAWSURFACE pSrc = (LPDIRECTDRAWSURFACE) src;
  RECT clientRect, backRect;
  FxBool status = FXTRUE;

  getClientRect((HWND)state->window_handle, &clientRect );
  backRect = clientRect;
  OffsetRect(&backRect, -backRect.left, -backRect.top );

  DD_TRY(state, status,
     pDst->Blt(&clientRect, pSrc,  &backRect, DDBLT_WAIT, NULL));

  return status;
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
   HRESULT hres;
   DDSURFACEDESC  desc;
   LPDIRECTDRAWSURFACE pSurface = (LPDIRECTDRAWSURFACE) surface;

   memset( &desc, 0, sizeof(desc));
   desc.dwSize = sizeof(desc);

   // grab the lock, try a few times.
   for(int i = 0; i < 10; i++){
     hres = pSurface->Lock(NULL, &desc, DDLOCK_WAIT, NULL);

     if (hres == DDERR_SURFACELOST) {
       pSurface->Restore();
       continue;
     }

     if(hres == DD_OK) {
       //printf("pf, h, w, pitch = %d, %d, %d, %d\n",
       //       desc.ddpfPixelFormat.dwRGBBitCount,
       //       desc.dwHeight, desc.dwWidth, desc.lPitch);
       return (void*) (desc.lpSurface);
     }
   }

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
   HRESULT hres;
   LPDIRECTDRAWSURFACE pSurface = (LPDIRECTDRAWSURFACE) surface;

   if (!ptr) {
     return;
   }

   // release the lock, try a few times
   for(int i = 0; i < 10; i++){
     hres = pSurface->Unlock(ptr);

     if (hres == DDERR_SURFACELOST) {
       pSurface->Restore();
       continue;
     }

     if(hres == DD_OK) {
       break;
     }
   }
}

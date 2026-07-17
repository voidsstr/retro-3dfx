/*
** Copyright (c) 1995-1998, 3Dfx Interactive, Inc.
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
** $Revision: 6$
** $Date: 10/11/00 8:57:13 PM$
**
*/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

#include "precomp.h"

/**************************************************************************
* P U B L I C   F U N C T I O N S
***************************************************************************/

DWORD __stdcall
DdDestroyPalette( LPDDHAL_DESTROYPALETTEDATA pdp )
{
  pdp->ddRVal = DD_OK;
  return DDHAL_DRIVER_NOTHANDLED;
}

DWORD __stdcall
DdSetEntries( LPDDHAL_SETENTRIESDATA pse )
{
  pse->ddRVal = DD_OK;
  return DDHAL_DRIVER_NOTHANDLED;
}

DWORD __stdcall
DdSetPalette( LPDDHAL_SETPALETTEDATA psp )
{
  psp->ddRVal = DD_OK;
  return DDHAL_DRIVER_NOTHANDLED;
}

DWORD __stdcall
DdCreatePalette( LPDDHAL_CREATEPALETTEDATA pcp )
{
  pcp->ddRVal = DD_OK;
  return DDHAL_DRIVER_NOTHANDLED;
}

DWORD __stdcall
DdSetDrvColorKey( LPDDHAL_DRVSETCOLORKEYDATA psdck )
{
  psdck->ddRVal = DD_OK;
  return DDHAL_DRIVER_NOTHANDLED;
}

#if DBG
#define NORMAL_DBG_LEVEL      2

VOID __stdcall
DUMP_SURFACEDESC( NT9XDEVICEDATA *ppdev, int Level, LPDDSURFACEDESC lpSD )
{
  DISPDBG((NORMAL_DBG_LEVEL, "  SurfaceDesc->dwFlags = %08lXh", lpSD->dwFlags));
  if ((DDSD_WIDTH | DDSD_HEIGHT) & lpSD->dwFlags)
    DISPDBG((NORMAL_DBG_LEVEL, "    size = %8lXh x %8lXh", lpSD->dwWidth,
                                            lpSD->dwHeight));
  else if (DDSD_WIDTH & lpSD->dwFlags)
    DISPDBG((NORMAL_DBG_LEVEL, "    width = %8lXh", lpSD->dwWidth));
  else if (DDSD_HEIGHT & lpSD->dwFlags)
    DISPDBG((NORMAL_DBG_LEVEL, "    height = %8lXh", lpSD->dwHeight));
  if (DDSD_BACKBUFFERCOUNT & lpSD->dwFlags)
    DISPDBG((NORMAL_DBG_LEVEL, "    backbuffer count = %8lXh", lpSD->dwBackBufferCount));
#if ENABLE_3D
  if (DDSD_MIPMAPCOUNT & lpSD->dwFlags)
    DISPDBG((NORMAL_DBG_LEVEL, "    mipmap count = %8lXh", lpSD->dwMipMapCount));
  if (DDSD_ZBUFFERBITDEPTH & lpSD->dwFlags)
    DISPDBG((NORMAL_DBG_LEVEL, "    zbuffer bit depth = %8lXh", lpSD->dwZBufferBitDepth));
#endif
  if (DDSD_PIXELFORMAT & lpSD->dwFlags)
  {
    DISPDBG((NORMAL_DBG_LEVEL, "    pixel format flags = %08lXh", lpSD->ddpfPixelFormat.dwFlags));
    if (DDPF_FOURCC & lpSD->ddpfPixelFormat.dwFlags)
      DISPDBG((NORMAL_DBG_LEVEL, "      fourcc = %08lXh (bitCount=%8lXh)",
               lpSD->ddpfPixelFormat.dwFourCC,
               lpSD->ddpfPixelFormat.dwRGBBitCount));
    if (DDPF_RGB & lpSD->ddpfPixelFormat.dwFlags)
    {
      DISPDBG((NORMAL_DBG_LEVEL, "      rgb bit count = %8lXh", lpSD->ddpfPixelFormat.dwRGBBitCount));
      DISPDBG((NORMAL_DBG_LEVEL, "      masks r=%8lXh g=%8lXh, b=%8lXh, a=%8lXh",
               lpSD->ddpfPixelFormat.dwRBitMask,
               lpSD->ddpfPixelFormat.dwGBitMask,
               lpSD->ddpfPixelFormat.dwBBitMask,
               lpSD->ddpfPixelFormat.dwRGBAlphaBitMask));
    }
#if ENABLE_3D
    if (DDPF_ZBUFFER & lpSD->ddpfPixelFormat.dwFlags)
    {
      DISPDBG((NORMAL_DBG_LEVEL, "      zbuffer bit depth = %8lXh", lpSD->ddpfPixelFormat.dwZBufferBitDepth));
      DISPDBG((NORMAL_DBG_LEVEL, "      mask = %8lXh", lpSD->ddpfPixelFormat.dwZBitMask));
    }
    if (DDPF_STENCILBUFFER & lpSD->ddpfPixelFormat.dwFlags)
    {
      DISPDBG((NORMAL_DBG_LEVEL, "      stencil bit depth = %8lXh", lpSD->ddpfPixelFormat.dwStencilBitDepth));
      DISPDBG((NORMAL_DBG_LEVEL, "      mask = %8lXh", lpSD->ddpfPixelFormat.dwStencilBitMask));
    }
    if (DDPF_LUMINANCE & lpSD->ddpfPixelFormat.dwFlags)
    {
      DISPDBG((NORMAL_DBG_LEVEL, "      luminance bit count = %8lXh", lpSD->ddpfPixelFormat.dwLuminanceBitCount));
      DISPDBG((NORMAL_DBG_LEVEL, "      mask = %8lXh", lpSD->ddpfPixelFormat.dwLuminanceBitMask));
    }
#endif
    if ((DDPF_ALPHAPIXELS | DDPF_ALPHA) & lpSD->ddpfPixelFormat.dwFlags)
    {
      DISPDBG((NORMAL_DBG_LEVEL, "      alpha bit count = %8lXh", lpSD->ddpfPixelFormat.dwAlphaBitDepth));
      DISPDBG((NORMAL_DBG_LEVEL, "      mask = %8lXh", lpSD->ddpfPixelFormat.dwRGBAlphaBitMask));
    }
#if ENABLE_3D
    if (DDPF_BUMPLUMINANCE & lpSD->ddpfPixelFormat.dwFlags)
    {
      DISPDBG((NORMAL_DBG_LEVEL, "      bump luminance bit count = %8lXh", lpSD->ddpfPixelFormat.dwLuminanceBitCount));
      DISPDBG((NORMAL_DBG_LEVEL, "      mask = %8lXh", lpSD->ddpfPixelFormat.dwLuminanceBitMask));
    }
    if (DDPF_BUMPDUDV & lpSD->ddpfPixelFormat.dwFlags)
    {
      DISPDBG((NORMAL_DBG_LEVEL, "      bump bit count = %8lXh", lpSD->ddpfPixelFormat.dwBumpBitCount));
      DISPDBG((NORMAL_DBG_LEVEL, "      masks  Du=%8lXh Dv=%8lXh BumpLuminence=%8lXh",
               lpSD->ddpfPixelFormat.dwBumpDuBitMask,
               lpSD->ddpfPixelFormat.dwBumpDvBitMask,
               lpSD->ddpfPixelFormat.dwBumpLuminanceBitMask));
    }
#endif
  }
}

VOID _stdcall
DUMP_SURFACEINFO(NT9XDEVICEDATA *ppdev, LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl)
{
  DISPDBG((NORMAL_DBG_LEVEL,"  pDDLcl->dwFlags = %08lXh", pDDSLcl->dwFlags));

  DISPDBG((NORMAL_DBG_LEVEL,"    fpVidMem = %8lXh", pDDSLcl->lpGbl->fpVidMem));
  DISPDBG((NORMAL_DBG_LEVEL,"    size     = %8lXh x %8lXh", pDDSLcl->lpGbl->wWidth,pDDSLcl->lpGbl->wHeight));
  DISPDBG((NORMAL_DBG_LEVEL,"    pitch = %8lXh", pDDSLcl->lpGbl->lPitch));

  DISPDBG((NORMAL_DBG_LEVEL,"    pixel format flags = %08lXh", pDDSLcl->lpGbl->ddpfSurface.dwFlags));
  if (DDPF_FOURCC & pDDSLcl->lpGbl->ddpfSurface.dwFlags)
    DISPDBG((NORMAL_DBG_LEVEL,"      fourcc = %08lXh (bitCount=%8lXh)",
             pDDSLcl->lpGbl->ddpfSurface.dwFourCC,
             pDDSLcl->lpGbl->ddpfSurface.dwRGBBitCount));
  if (DDPF_RGB & pDDSLcl->lpGbl->ddpfSurface.dwFlags)
  {
    DISPDBG((NORMAL_DBG_LEVEL, "      rgb bit count = %8lXh", pDDSLcl->lpGbl->ddpfSurface.dwRGBBitCount));
    DISPDBG((NORMAL_DBG_LEVEL, "      masks r=%8lXh g=%8lXh, b=%8lXh, a=%8lXh",
             pDDSLcl->lpGbl->ddpfSurface.dwRBitMask,
             pDDSLcl->lpGbl->ddpfSurface.dwGBitMask,
             pDDSLcl->lpGbl->ddpfSurface.dwBBitMask,
             pDDSLcl->lpGbl->ddpfSurface.dwRGBAlphaBitMask));
  }
#if ENABLE_3D
  if (DDPF_ZBUFFER & pDDSLcl->lpGbl->ddpfSurface.dwFlags)
  {
    DISPDBG((NORMAL_DBG_LEVEL, "      zbuffer bit depth = %8lXh", pDDSLcl->lpGbl->ddpfSurface.dwZBufferBitDepth));
    DISPDBG((NORMAL_DBG_LEVEL, "      mask = %8lXh", pDDSLcl->lpGbl->ddpfSurface.dwZBitMask));
  }
  if (DDPF_STENCILBUFFER & pDDSLcl->lpGbl->ddpfSurface.dwFlags)
  {
    DISPDBG((NORMAL_DBG_LEVEL, "      stencil bit depth = %8lXh", pDDSLcl->lpGbl->ddpfSurface.dwStencilBitDepth));
    DISPDBG((NORMAL_DBG_LEVEL, "      mask = %8lXh", pDDSLcl->lpGbl->ddpfSurface.dwStencilBitMask));
  }
  if (DDPF_LUMINANCE & pDDSLcl->lpGbl->ddpfSurface.dwFlags)
  {
    DISPDBG((NORMAL_DBG_LEVEL, "      luminance bit count = %8lXh", pDDSLcl->lpGbl->ddpfSurface.dwLuminanceBitCount));
    DISPDBG((NORMAL_DBG_LEVEL, "      mask = %8lXh", pDDSLcl->lpGbl->ddpfSurface.dwLuminanceBitMask));
  }
#endif
  if ((DDPF_ALPHAPIXELS | DDPF_ALPHA) & pDDSLcl->lpGbl->ddpfSurface.dwFlags)
  {
    DISPDBG((NORMAL_DBG_LEVEL, "      alpha bit count = %8lXh", pDDSLcl->lpGbl->ddpfSurface.dwAlphaBitDepth));
    DISPDBG((NORMAL_DBG_LEVEL, "      mask = %8lXh", pDDSLcl->lpGbl->ddpfSurface.dwRGBAlphaBitMask));
  }
#if ENABLE_3D
  if (DDPF_BUMPLUMINANCE & pDDSLcl->lpGbl->ddpfSurface.dwFlags)
  {
    DISPDBG((NORMAL_DBG_LEVEL, "      bump luminance bit count = %8lXh", pDDSLcl->lpGbl->ddpfSurface.dwLuminanceBitCount));
    DISPDBG((NORMAL_DBG_LEVEL, "      mask = %8lXh", pDDSLcl->lpGbl->ddpfSurface.dwLuminanceBitMask));
  }
  if (DDPF_BUMPDUDV & pDDSLcl->lpGbl->ddpfSurface.dwFlags)
  {
    DISPDBG((NORMAL_DBG_LEVEL, "      bump bit count = %8lXh", pDDSLcl->lpGbl->ddpfSurface.dwBumpBitCount));
    DISPDBG((NORMAL_DBG_LEVEL, "      masks  Du=%8lXh Dv=%8lXh BumpLuminence=%8lXh",
             pDDSLcl->lpGbl->ddpfSurface.dwBumpDuBitMask,
             pDDSLcl->lpGbl->ddpfSurface.dwBumpDvBitMask,
             pDDSLcl->lpGbl->ddpfSurface.dwBumpLuminanceBitMask));
  }
#endif
}
#endif


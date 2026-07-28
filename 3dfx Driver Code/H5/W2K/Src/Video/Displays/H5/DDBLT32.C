/* $Header: ddblt32.c, 51, 10/25/00 4:57:29 AM, Johnny Trainor $ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File Name:   DDBLT32.C
**
** Description: DirectDraw bitblt related functions
**
** $Revision: 51$
** $Date: 10/25/00 4:57:29 AM$
**
*/

/****************************************************************************
*
* DIRECTDRAW FUNCTIONS:
*
* DdGetBltStatus        --- DirectDraw GetBltStatus entry point.
* DdBlt                 --- DirectDraw Blt entry point.
*
* INTERNAL FUNCTIONS:
*
* Blt32_ColorFill       --- fill surface with a color, always use SRCCOPY ROP
* Blt32_DoBltNoSP       --- blt w/o source/pattern, may have dest. color key
* Blt32_DoBltS          --- screen to screen blt, also handle stretch/shrink
* Blt32_SystemToVideo   --- perform host to screen blt
*
****************************************************************************/

#include "precomp.h"
#if ENABLE_3D && !defined(WINNT)
#include "d3txtr.h"
#endif

#if ENABLE_3D
#ifdef WINNT
#define UNPAD_DXT1(arg1,arg2)   unpad_dxt1(arg1,arg2)
#define PAD_DXT1(arg1,arg2)     pad_dxt1(arg1,arg2)
#else
#define UNPAD_DXT1(arg1,arg2)   unpad_dxt1(arg2)
#define PAD_DXT1(arg1,arg2)     pad_dxt1(arg2)
#endif

void UNPAD_DXT1(NT9XDEVICEDATA*,LPDDRAWI_DDRAWSURFACE_GBL surfGBL);
void PAD_DXT1(NT9XDEVICEDATA*,LPDDRAWI_DDRAWSURFACE_GBL surfGBL);
#endif

#if defined(SLI_AA) && ENABLE_3D
#include <ddsli2d.h>
#endif


/***************************************************************************/
/*                             DEFINES                                     */
/***************************************************************************/
/*
 * ROP utility macros. These take a standard rop byte.
 */
#define ROP_HAS_SRC(rop3) (0 != (0x33 & ( (ULONG)rop3 ^ ((ULONG)rop3 >> 2))))
#define ROP_HAS_PAT(rop3) (0 != (0x0F & ( (ULONG)rop3 ^ ((ULONG)rop3 >> 4))))
#define ROP_HAS_DST(rop3) (0 != (0x55 & ( (ULONG)rop3 ^ ((ULONG)rop3 >> 1))))

/***************************************************************************/
/*                       DIRECTDRAW FUNCTIONS                              */
/***************************************************************************/

/*----------------------------------------------------------------------
Function name: DdGetBltStatus

Description:   DDRAW Callback GetBltStatus()

               Determines if the blitter queue has room for more blits
               and if the blitter is busy.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
DdGetBltStatus( LPDDHAL_GETBLTSTATUSDATA lpGetBltStatus )
{
  DD_ENTRY_SETUP(lpGetBltStatus->lpDD);

  DDPRINT(DDDBGLVL, ">> DdGetBltStatus");

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "GetBltStatus32" ));
  DUMP_DDHAL_GETBLTSTATUSDATA(ppdev, DEBUG_DDGORY, lpGetBltStatus );
  #endif

  // Ignore device status and return DD_OK for 3D applications,
  // otherwise return DDERR_WASSTILLDRAWING if device busy.

  lpGetBltStatus->ddRVal = DD_OK;

  if (!_DD(dd3DInOverlay))
  {
    if (lpGetBltStatus->dwFlags == DDGBS_ISBLTDONE)
    {
      if (FXGETBUSYSTATUS(ppdev))
          {
        lpGetBltStatus->ddRVal = DDERR_WASSTILLDRAWING;
          }
    }
  }

  DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", lpGetBltStatus->ddRVal);

  return DDHAL_DRIVER_HANDLED;

} /* DdGetBltStatus */


/*----------------------------------------------------------------------
Function name: DdBlt

Description:   DDRAW 32 bit Callback Blt32()

               Handles Blt color fill, system to video memory,
                           and video to video memory transfers.  Supports
                           source and destination color keying, scaling,
                           and color format conversion.


Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
                           DDHAL_DRIVER_NOTHANDLED
                           DD_OK
----------------------------------------------------------------------*/

DWORD __stdcall
DdBlt( LPDDHAL_BLTDATA pbd )
{
  DWORD                rop3, dst_dwCaps, src_dwCaps;
  DWORD                srcPixelFormat, dstPixelFormat;
  DWORD                pixelByteDepth, dwBltFlags;
#ifndef WINNT
  DWORD                dwDestSurfFlags, dwSrcSurfFlags;
#endif
#if defined(WINNT) && (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  DWORD                dwSrcTxHndl, dwDstTxHndl;
#endif
  RC                   *pRc;

  DD_ENTRY_SETUP(pbd->lpDD);

  pRc = (RC *)_D3(lastContext);
  DDPRINT(DDDBGLVL, ">> DdBlt (dst=%08lXh, src=%08lXh", pbd->lpDDDestSurface, pbd->lpDDSrcSurface);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "Blt32" ));
  DUMP_BLTDATA(ppdev, DEBUG_DDGORY, pbd );
  #endif

  // Fetch surface capabilities and blt flags.

  if (pbd->lpDDDestSurface)
    dst_dwCaps = pbd->lpDDDestSurface->ddsCaps.dwCaps;

  if (pbd->lpDDSrcSurface)
    src_dwCaps = pbd->lpDDSrcSurface->ddsCaps.dwCaps;

#ifdef Z_ACCESS_OPT
  // If we are blt'ing to clear the Z Buffer, lets reset the optimization
  if ( _DD(ddEnableZClearOpt) && (dst_dwCaps & DDSCAPS_ZBUFFER) &&
       _DD(dd3DInOverlay) && (_FF(dd3DSurfaceCount) > 2) && (_FF(dd3DSurfaceCount) < 5) )
  {
    if ( ((pRc)&&(pRc->dwZClearOptEnabled)) || ((_DD(ddFlipsWithoutZClear) & 0x7fffffff) > 6) )
    {	
      _DD(ddFlipsWithoutZClear) = 0x80000000;
    }
    else
    {
      // Don't overwrite a reset status.
      if ( _DD(ddFlipsWithoutZClear) != 0x80000000 )
        _DD(ddFlipsWithoutZClear) = 0;
    }
  }
#endif

  dwBltFlags = pbd->dwFlags;

  // Cannot handle destination in system memory.

  if( dst_dwCaps & DDSCAPS_SYSTEMMEMORY)
  {
    goto unsupported;
  }

  _DD(ddAcceleratorUsed) = 1;  // Notify flipping code.

  // Check destination pixel format.

#ifndef WINNT
  // Note that unlike Windows 95, Windows NT always guarantees that
  // there will be a valid 'ddpfSurface' structure.
  dwDestSurfFlags = pbd->lpDDDestSurface->dwFlags;
  if(dwDestSurfFlags & DDRAWISURF_HASPIXELFORMAT)
  {
#endif
    dstPixelFormat = INVALID_PIXELFORMAT;  // Assume format not handled.

#if ENABLE_3D
    if (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFlags & (DDPF_RGB | DDPF_LUMINANCE | DDPF_ALPHAPIXELS ))
#else
    if (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFlags & DDPF_RGB)
#endif
    {
      pixelByteDepth = (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwRGBBitCount ) >> 3;
      GETPIXELFORMAT(pixelByteDepth, dstPixelFormat);
    }
#if ENABLE_3D
    else if (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFlags & DDPF_ZBUFFER)
    {
      pixelByteDepth = (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwZBufferBitDepth ) >> 3;
      GETPIXELFORMAT(pixelByteDepth, dstPixelFormat);
    }
#endif
    else if (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFlags & DDPF_FOURCC)
    {
      if (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC==FOURCC_YUY2)
      {
        pixelByteDepth = (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwYUVBitCount ) >> 3;
        if (pixelByteDepth == 2)
          dstPixelFormat = SSTG_PIXFMT_422YUV;
      }
      else if (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC==FOURCC_UYVY)
      {
        pixelByteDepth = (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwYUVBitCount ) >> 3;
        if (pixelByteDepth == 2)
          dstPixelFormat = SSTG_PIXFMT_422UYV;
      }
#if ENABLE_3D
      // check for DXTn and FXT1 formats
      else if ((IS_NAPALM) &&
               ((FOURCC_DXT1 == pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC) ||
                (FOURCC_DXT2 == pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC) ||
                (FOURCC_DXT3 == pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC) ||
                (FOURCC_DXT4 == pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC) ||
                (FOURCC_DXT5 == pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC) ||
                (FOURCC_FXT1 == pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC)))
      {
        dstPixelFormat = pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC;
      }
#endif
#ifdef SIMULATE_YV12
      else if (FOURCC_YV12 == pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC)
      {
        dstPixelFormat = FOURCC_YV12;
      }
#endif
    }
#ifndef WINNT
  }
  else
  {
    pixelByteDepth = GETPRIMARYBYTEDEPTH; // Primary or Zbuffer depth are equal
    GETPIXELFORMAT(pixelByteDepth, dstPixelFormat);
  }
#endif // ifndef WINNT

  if (dstPixelFormat == INVALID_PIXELFORMAT)
  {
    goto unsupported;
  }

  // Color Fill

#if ENABLE_3D
  if (dwBltFlags & (DDBLT_COLORFILL | DDBLT_DEPTHFILL))
#else
  if (dwBltFlags & DDBLT_COLORFILL)
#endif
  {
    // Hardware cannot handle YUV destination, use 16bpp format instead!

    if ((SSTG_PIXFMT_422YUV == dstPixelFormat) || (SSTG_PIXFMT_422UYV == dstPixelFormat))
    {
      dstPixelFormat = SSTG_PIXFMT_16BPP;
    }
#if ENABLE_3D
    // these should never happen! but what the hell
    else if ((FOURCC_DXT1 == dstPixelFormat) ||
             (FOURCC_DXT2 == dstPixelFormat) ||
             (FOURCC_DXT3 == dstPixelFormat) ||
             (FOURCC_DXT4 == dstPixelFormat) ||
             (FOURCC_DXT5 == dstPixelFormat) ||
             (FOURCC_FXT1 == dstPixelFormat))
    {
      //_asm int 3;
      goto unsupported;
    }
#endif
#ifdef SIMULATE_YV12
    else if (FOURCC_YV12 == pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC)
    {
      //_asm int 3;
      goto unsupported;
    }
#endif

    if (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_TEXTURE)
    {
      TXTRDESC *txtr;
      txtr = TXTRDESC_PTR(pbd->lpDDDestSurface->dwReserved1);

      txtr->flags |= BltToTxtrInFifo;
    }

#if defined(SLI_AA) && (!defined(WINNT) || (_WIN32_WINNT >= 0x0500))
    if ( _DD(ddSLIModeEnabled) || _DD(ddAAModeEnabled) )
#ifdef SLI_AA_2D
      return( DdSliColorFill(ppdev, pbd, dstPixelFormat, dst_dwCaps) );
#else
      if (dst_dwCaps & DDSCAPS_ZBUFFER)
        return( sli_ColorFillZ(ppdev, pbd, pixelByteDepth) );
      else
        return( sli_ColorFill(ppdev, pbd, pixelByteDepth) );
#endif
    else if (_DD(ddAANumberSamples))
         return (Dd2SampleColorFill(ppdev, pbd, dstPixelFormat));
    else
#endif
      return (Blt32_ColorFill(ppdev, pbd, dstPixelFormat));
  }

  rop3 = (DDBLT_ROP & pbd->dwFlags) ? HIWORD(pbd->bltFX.dwROP) : HIWORD(SRCCOPY);

  // Whiteness and Blackness rops

  if (!ROP_HAS_SRC(rop3))
  {
#if defined(SLI_AA) && (!defined(WINNT) || (_WIN32_WINNT >= 0x0500))
    if ( _DD(ddSLIModeEnabled) || _DD(ddAAModeEnabled) )
#ifdef SLI_AA_2D
      return (DdSliBltNoSP(ppdev, pbd, rop3, dstPixelFormat));
#else
      return (sli_DoBltNoSP(ppdev, pbd, rop3, pixelByteDepth));
#endif
    else if (_DD(ddAANumberSamples))
         return (Dd2SampleBltNoSP(ppdev, pbd, rop3, dstPixelFormat));
    else
#endif
      return (Blt32_DoBltNoSP(ppdev, pbd, rop3, dstPixelFormat));
  }

  // Check source pixel format.

#ifndef WINNT
  // Note that unlike Windows 95, Windows NT always guarantees that
  // there will be a valid 'ddpfSurface' structure.
  dwSrcSurfFlags = pbd->lpDDSrcSurface->dwFlags;
  if(dwSrcSurfFlags & DDRAWISURF_HASPIXELFORMAT)
  {
#endif
    srcPixelFormat = INVALID_PIXELFORMAT;  // Assume format not handled.

#if ENABLE_3D
    if (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFlags & (DDPF_RGB | DDPF_LUMINANCE | DDPF_ALPHAPIXELS ))
#else
    if (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFlags & DDPF_RGB)
#endif
    {
      pixelByteDepth = (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwRGBBitCount ) >> 3;
      GETPIXELFORMAT(pixelByteDepth, srcPixelFormat);
    }
#if ENABLE_3D
    else if (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFlags & DDPF_ZBUFFER)
    {
      pixelByteDepth = (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwZBufferBitDepth ) >> 3;
      GETPIXELFORMAT(pixelByteDepth, srcPixelFormat);
    }
#endif
    else if (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFlags & DDPF_FOURCC)
    {
      if (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC==FOURCC_YUY2)
      {
        pixelByteDepth = (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwYUVBitCount ) >> 3;
        if (pixelByteDepth == 2)
          srcPixelFormat = SSTG_PIXFMT_422YUV;
      }
      else if (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC==FOURCC_UYVY)
      {
        pixelByteDepth = (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwYUVBitCount ) >> 3;
        if (pixelByteDepth == 2)
          srcPixelFormat = SSTG_PIXFMT_422UYV;
      }
#if ENABLE_3D
      // check for DXTn and FXT1 formats
      else if ((IS_NAPALM) &&
               ((FOURCC_DXT1 == pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC) ||
                (FOURCC_DXT2 == pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC) ||
                (FOURCC_DXT3 == pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC) ||
                (FOURCC_DXT4 == pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC) ||
                (FOURCC_DXT5 == pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC) ||
                (FOURCC_FXT1 == pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC)))
      {
        srcPixelFormat = pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC;
      }
#endif
#ifdef SIMULATE_YV12
      else if (FOURCC_YV12 == pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC)
      {
        srcPixelFormat = FOURCC_YV12;
      }
#endif
    }
#ifndef WINNT
  }
  else
  {
    pixelByteDepth = GETPRIMARYBYTEDEPTH; // Primary or Zbuffer depth are equal
    GETPIXELFORMAT(pixelByteDepth, srcPixelFormat);
  }
#endif // ifndef WINNT

  if (srcPixelFormat ==  INVALID_PIXELFORMAT)
  {
    goto unsupported;
  }

  // Check for cases not handled by hardware.

  if ((SSTG_PIXFMT_422YUV == dstPixelFormat) || (SSTG_PIXFMT_422UYV == dstPixelFormat))
  {
    // Allow YUY2-YUY2 or UYVY-UYVY, tell the hardware format is RGB565

    if (srcPixelFormat == dstPixelFormat)
    {
      srcPixelFormat =
      dstPixelFormat = SSTG_PIXFMT_16BPP;
    }
    // Fail YUY2-UYVY and UYVY-YUY2, cannot handle color conversion.

    else
    {
      goto unsupported;
    }
  }
#if ENABLE_3D
  else if ((FOURCC_DXT1 == dstPixelFormat) ||
           (FOURCC_DXT2 == dstPixelFormat) ||
           (FOURCC_DXT3 == dstPixelFormat) ||
           (FOURCC_DXT4 == dstPixelFormat) ||
           (FOURCC_DXT5 == dstPixelFormat) ||
           (FOURCC_FXT1 == dstPixelFormat) ||
           (FOURCC_DXT1 == srcPixelFormat) ||
           (FOURCC_DXT2 == srcPixelFormat) ||
           (FOURCC_DXT3 == srcPixelFormat) ||
           (FOURCC_DXT4 == srcPixelFormat) ||
           (FOURCC_DXT5 == srcPixelFormat) ||
           (FOURCC_FXT1 == srcPixelFormat))
  {
    if (srcPixelFormat == dstPixelFormat)
    {
      pbd->ddRVal = Blt32_CopyFourCC(ppdev, pbd->lpDDSrcSurface, &pbd->rSrc, pbd->lpDDDestSurface, &pbd->rDest);
      DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pbd->ddRVal);
      return DDHAL_DRIVER_HANDLED;
    }
    else
      goto unsupported;
  }
#endif
#ifdef SIMULATE_YV12
  else if ((FOURCC_YV12 == dstPixelFormat) || (FOURCC_YV12 == srcPixelFormat))
  {
    if (srcPixelFormat == dstPixelFormat)
    {
      // blt the YV12 region of src to dst
      pbd->ddRVal = Blt32_CopyFourCC(ppdev, pbd->lpDDSrcSurface, &pbd->rSrc, pbd->lpDDDestSurface, &pbd->rDest);
      // fall thru and blt the YUY2 region of src to dst
      srcPixelFormat = dstPixelFormat = SSTG_PIXFMT_16BPP;
    }
    else
      goto unsupported;
  }
#endif

  // Hardware cannot handle color conversion to/from 8bpp.

  if ((srcPixelFormat != dstPixelFormat) && ((SSTG_PIXFMT_8BPP == dstPixelFormat) || (SSTG_PIXFMT_8BPP == srcPixelFormat)))
  {
    goto unsupported;
  }

#if ENABLE_3D

  // Handle texture download, only if both surfaces are textures, and blt is system to video memory.

  if ((src_dwCaps & DDSCAPS_TEXTURE) && (src_dwCaps & DDSCAPS_SYSTEMMEMORY) &&
      (dst_dwCaps & DDSCAPS_TEXTURE) && (dst_dwCaps & DDSCAPS_VIDEOMEMORY))
  {
    // Check that width & height are powers of 2.

    if (((pbd->lpDDSrcSurface->lpGbl->wWidth  & ~(pbd->lpDDSrcSurface->lpGbl->wWidth -1)) != pbd->lpDDSrcSurface->lpGbl->wWidth) ||
        ((pbd->lpDDSrcSurface->lpGbl->wHeight & ~(pbd->lpDDSrcSurface->lpGbl->wHeight-1)) != pbd->lpDDSrcSurface->lpGbl->wHeight))
    {
      #ifndef WINNT
      DISPDBG((ppdev, DEBUG_DDDETAILS,"Blt32: bad texture size, not a power of 2, w=%d, h=%d\n",
               pbd->lpDDSrcSurface->lpGbl->wWidth, pbd->lpDDSrcSurface->lpGbl->wHeight));
      #else
      DDPRINT(DDDBGLVL,"Blt32: bad texture size, not a power of 2, w=%d, h=%d\n",
              pbd->lpDDSrcSurface->lpGbl->wWidth, pbd->lpDDSrcSurface->lpGbl->wHeight);
      #endif
      pbd->ddRVal = DDERR_GENERIC;
      return DDHAL_DRIVER_HANDLED ;
    }

    // retro3dfx: resolve the texture-download handle table CONTEXT-INDEPENDENTLY.
    //
    // The original code resolved both surface handles with TXTRHNDL_PTR(), which on
    // DX7/NT expands to  pRc->pHndlList->ppTxtrHndlList[h]  where pRc = _D3(lastContext).
    // That crashed GoldSrc's Direct3D renderer: _D3(lastContext) is set ONLY inside the
    // DrawPrimitives2 draw path and is ZEROED by textureLoad() itself, so a texture
    // upload -- which precedes the first draw, or follows a prior upload in the same
    // batch -- legitimately runs with lastContext == NULL, and pRc->pHndlList then
    // dereferenced [NULL+0x510] -> kernel AV (bugcheck 0x8E, DdBlt+0x32C).
    //
    // On W2K/NT a surface carries no owning DirectDraw-local, so we can't key the
    // handle list off the surface.  Resolve through the GLOBAL per-DDLcl handle-list
    // chain (g_pHndlList, D7D3D.C -- the list GetHndlListPtr iterates).  It is
    // populated at ddiCreateSurfaceEx time and exists INDEPENDENT of any render
    // context: GoldSrc restarts its video mode several times at startup, and
    // uploads issued in the context-less windows between CTX-DESTROY and the next
    // CTX-CREATE must still resolve (walking g_pContexts dropped those uploads --
    // ring 'TEXDL-SKIP no-RC' -- leaving stale-white texture memory).  Find the
    // HNDLLIST that validly resolves BOTH handles and load through its TXTRHNDLs.
    // textureLoad() needs only ppdev + the two TXTRHNDLs (its body reads no
    // context state).  goto unsupported only when no list resolves the handles
    // (worst case == the old guard-only behaviour, never a fault).
#if defined(WINNT) && (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    {
      extern HNDLLIST *g_pHndlList;   // D7D3D.C: head of the per-DDLcl handle lists
      HNDLLIST *pHL;
      HNDLLIST *pTxHndlList = NULL;
      TXTRHNDL  *pSrcTxtr = NULL, *pDstTxtr = NULL;
      LPDDRAWI_DDRAWSURFACE_LCL pSrcRoot, pDstRoot;
      int       nSrcLvl = 0, nDstLvl = 0, nHops;

      if ( !pbd->lpDDSrcSurface->lpSurfMore || !pbd->lpDDDestSurface->lpSurfMore )
      {
        DDPRINT(DDDBGLVL, "Blt32 - texture download skipped: surface has no lpSurfMore");
        goto unsupported;
      }

      // Mip SUBLEVEL surfaces carry their own dwSurfaceHandle that
      // ddiCreateSurfaceEx never registers (only the chain ROOT gets a
      // TXTRHNDL, with the whole chain described in mmData[]).  GoldSrc blts
      // every mip level as a separate surface; resolving the sublevel handle
      // directly fails, which used to silently drop mips 1..n of every world
      // texture (ring TEXDL-SKIP) -> the TMU minified into stale-white memory.
      // Walk each surface UP the attach-from chain to its texture root and
      // resolve THAT handle; the level index is recovered below by matching
      // the blitted surface's dimensions against the root's mmData[] (the
      // same idiom the DP2 TEXBLT mip-match loop uses).
      pSrcRoot = pbd->lpDDSrcSurface;
      for ( nHops = 0; nHops < MAX_MIPMAP_LEVELS; nHops++ )
      {
        if ( (NULL == pSrcRoot->lpAttachListFrom) ||
             (NULL == pSrcRoot->lpAttachListFrom->lpAttached) ||
             !(pSrcRoot->lpAttachListFrom->lpAttached->ddsCaps.dwCaps & DDSCAPS_TEXTURE) )
          break;
        pSrcRoot = pSrcRoot->lpAttachListFrom->lpAttached;
      }
      pDstRoot = pbd->lpDDDestSurface;
      for ( nHops = 0; nHops < MAX_MIPMAP_LEVELS; nHops++ )
      {
        if ( (NULL == pDstRoot->lpAttachListFrom) ||
             (NULL == pDstRoot->lpAttachListFrom->lpAttached) ||
             !(pDstRoot->lpAttachListFrom->lpAttached->ddsCaps.dwCaps & DDSCAPS_TEXTURE) )
          break;
        pDstRoot = pDstRoot->lpAttachListFrom->lpAttached;
      }
      if ( !pSrcRoot->lpSurfMore || !pDstRoot->lpSurfMore )
      {
        DDPRINT(DDDBGLVL, "Blt32 - texture download skipped: root has no lpSurfMore");
        goto unsupported;
      }
      dwSrcTxHndl = pSrcRoot->lpSurfMore->dwSurfaceHandle;
      dwDstTxHndl = pDstRoot->lpSurfMore->dwSurfaceHandle;
      if ( (0 == dwSrcTxHndl) || (0 == dwDstTxHndl) )
      {
        DDPRINT(DDDBGLVL, "Blt32 - texture download skipped: zero surface handle");
        goto unsupported;
      }

      for ( pHL = g_pHndlList; NULL != pHL; pHL = pHL->pNext )
      {
        // valid list, both handles in range (ppTxtrHndlList[0] holds the count,
        // handle <= count -- matches D3TXTR.C:1218), and both registered.
        if ( NULL == pHL->ppTxtrHndlList )
          continue;
        if ( (dwSrcTxHndl > (DWORD)pHL->ppTxtrHndlList[0]) ||
             (dwDstTxHndl > (DWORD)pHL->ppTxtrHndlList[0]) )
          continue;
        if ( (NULL != pHL->ppTxtrHndlList[dwSrcTxHndl]) &&
             (NULL != pHL->ppTxtrHndlList[dwDstTxHndl]) )
        {
          pTxHndlList = pHL;
          pSrcTxtr    = pHL->ppTxtrHndlList[dwSrcTxHndl];
          pDstTxtr    = pHL->ppTxtrHndlList[dwDstTxHndl];
          break;
        }
      }

      if ( NULL == pTxHndlList )
      {
        DDPRINT(DDDBGLVL, "Blt32 - texture download skipped: no context resolves handles (src=%ld dst=%ld)",
                dwSrcTxHndl, dwDstTxHndl);
#if ENABLE_LOG_FILE
        // white-texture hunt: a silent skip here means the app's upload fell back
        // to a software copy into tiled vidmem. Bounded: first 4 only.
        {
          static DWORD _dlSkip = 0;
          if (++_dlSkip <= 4)
            retroLogForce(ppdev, "retro3dfx TEXDL-SKIP#%ld: no-HL src=%ld dst=%ld %ldx%ld\r\n",
                          _dlSkip, dwSrcTxHndl, dwDstTxHndl,
                          (LONG)pbd->lpDDSrcSurface->lpGbl->wWidth,
                          (LONG)pbd->lpDDSrcSurface->lpGbl->wHeight);
        }
#endif
        goto unsupported;
      }

      // Recover the mip level being blitted: match the ORIGINAL surface's
      // dimensions against the root TXTRHNDL's per-LOD mmData[].  A top-level
      // blt (surface == its own root) is LOD 0 by definition.
      if ( pSrcRoot != pbd->lpDDSrcSurface )
      {
        for ( nSrcLvl = 0; nSrcLvl < pSrcTxtr->nLevels; nSrcLvl++ )
          if ( (pSrcTxtr->mmData[nSrcLvl].wWidth  == (DWORD)pbd->lpDDSrcSurface->lpGbl->wWidth) &&
               (pSrcTxtr->mmData[nSrcLvl].wHeight == (DWORD)pbd->lpDDSrcSurface->lpGbl->wHeight) )
            break;
        if ( nSrcLvl >= pSrcTxtr->nLevels )
        {
          DDPRINT(DDDBGLVL, "Blt32 - texture download skipped: src sublevel not in mmData");
          goto unsupported;
        }
      }
      if ( pDstRoot != pbd->lpDDDestSurface )
      {
        for ( nDstLvl = 0; nDstLvl < pDstTxtr->nLevels; nDstLvl++ )
          if ( (pDstTxtr->mmData[nDstLvl].wWidth  == (DWORD)pbd->lpDDDestSurface->lpGbl->wWidth) &&
               (pDstTxtr->mmData[nDstLvl].wHeight == (DWORD)pbd->lpDDDestSurface->lpGbl->wHeight) )
            break;
        if ( nDstLvl >= pDstTxtr->nLevels )
        {
          DDPRINT(DDDBGLVL, "Blt32 - texture download skipped: dst sublevel not in mmData");
          goto unsupported;
        }
      }

      DDPRINT(DDDBGLVL, "Blt32 - Texture download (ctx-independent, lod %d->%d)", nSrcLvl, nDstLvl);
      pbd->ddRVal = TEXTURELOAD(ppdev, pSrcTxtr, &pbd->rSrc, nSrcLvl, pDstTxtr, &pbd->rDest, nDstLvl);
#if ENABLE_LOG_FILE
      // white-texture hunt: log the first 3 downloads (+ every 256th) with the
      // mip count and pixel format so we can see what GoldSrc actually uploads.
      {
        static DWORD _dlOk = 0;
        ++_dlOk;
        // first 2 + rare heartbeat only: the bind-side logs need the ring slots
        // log the first few LOD-0 AND the first few sublevel downloads
        {
          static DWORD _dlSub = 0;
          DWORD _isSub = (nSrcLvl | nDstLvl) ? 1 : 0;
          if (_isSub) ++_dlSub;
          if ((_dlOk <= 2) || (0 == (_dlOk & 8191)) || (_isSub && (_dlSub <= 3)))
            retroLogForce(ppdev, "retro3dfx TEXDL#%ld: hr=%08lXh src=%ld dst=%ld %ldx%ld lod=%d/%d nlv=%d\r\n",
                          _dlOk, pbd->ddRVal, dwSrcTxHndl, dwDstTxHndl,
                          (LONG)pbd->lpDDSrcSurface->lpGbl->wWidth,
                          (LONG)pbd->lpDDSrcSurface->lpGbl->wHeight,
                          nSrcLvl, nDstLvl, pDstTxtr->nLevels);
        }
      }
#endif
      return DDHAL_DRIVER_HANDLED;
    }
#else
    DISPDBG((ppdev, DEBUG_DDDETAILS,"Blt32 - Texture download"));
    pbd->ddRVal = TEXTURELOAD(ppdev,
                              TXTRHNDL_PTR(pbd->lpDDSrcSurface->lpSurfMore->dwSurfaceHandle),
                              &pbd->rSrc, 0,
                              TXTRHNDL_PTR(pbd->lpDDDestSurface->lpSurfMore->dwSurfaceHandle),
                              &pbd->rDest, 0);

    return DDHAL_DRIVER_HANDLED;
#endif
  }
#endif

  // Video to Video

  if ((src_dwCaps & DDSCAPS_VIDEOMEMORY) && (dst_dwCaps & DDSCAPS_VIDEOMEMORY))
  {
#if defined(SLI_AA) && (!defined(WINNT) || (_WIN32_WINNT >= 0x0500))
    if ( _DD(ddSLIModeEnabled) || _DD(ddAAModeEnabled) )
#ifdef SLI_AA_2D
      return (DdSli2DScn2Scn(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat));
#else
      return (sli_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat, vid2vid));
#endif
    else if (_DD(ddAANumberSamples))
         return (Dd2SampleBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat));
    else
#endif
      return (Blt32_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat));
  }

  // Video to System

  else if ((src_dwCaps & DDSCAPS_SYSTEMMEMORY) && (dst_dwCaps & DDSCAPS_VIDEOMEMORY))
  {
#if defined(SLI_AA) && (!defined(WINNT) || (_WIN32_WINNT >= 0x0500))
    if ( _DD(ddSLIModeEnabled) || _DD(ddAAModeEnabled) )
#ifdef SLI_AA_2D
      return (DdSli2DSystem2Scn(ppdev, pbd, rop3, srcPixelFormat, pixelByteDepth, dstPixelFormat));
#else
      return (sli_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat, mem2vid));
#endif
    else if (_DD(ddAANumberSamples))
         return (Dd2SampleSystemToVideo(ppdev, pbd, rop3, srcPixelFormat, pixelByteDepth, dstPixelFormat));
    else
#endif
      return (Blt32_SystemToVideo(ppdev, pbd, rop3, srcPixelFormat, pixelByteDepth, dstPixelFormat));
  }

unsupported:

  pbd->ddRVal = DDERR_UNSUPPORTED;
  DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pbd->ddRVal);
  return DDHAL_DRIVER_HANDLED;

} /* DdBlt */


/***************************************************************************/
/*                        INTERNAL FUNCTIONS                               */
/***************************************************************************/

/*----------------------------------------------------------------------
Function name:  Blt32_ColorFill

Description:    Fill a rectangular region of a surface

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
Blt32_ColorFill(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD dstPixelFormat)
{
  DWORD     bltDstBaseAddr, bltDstFormat, bltDstSize, bltRop, bltDstXY;
  RECTL     dstRect = pbd->rDest;
  long      dstLeft, dstTop, dstRight, dstBottom;
  long      dstWidth, dstHeight, dstPitch;
  DWORD     packetHeader = 0, bumpNum=10, clip1min, clip1max;
#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
  DWORD     dwFillColor;
#endif
  CMDFIFO_PROLOG(hwPtr);

  dstTop = dstRect.top;
  dstRight = dstRect.right;
  dstBottom = dstRect.bottom;
  dstLeft = dstRect.left;

  dstWidth = dstRight - dstLeft;
  dstHeight = dstBottom - dstTop;

  bltDstBaseAddr = GET_HW_ADDR(pbd->lpDDDestSurface);
#if ENABLE_TILED_HEAP
  if(IS_TILED(bltDstBaseAddr))
  {
    dstPitch = _DS(ddTileStride);
  }
  else
#endif
  {
    dstPitch = pbd->lpDDDestSurface->lpGbl->lPitch;
  }

#if defined(WINNT) && (_WIN32_WINNT < 0x0500)
  // make space to restore nt invariant regs
  bumpNum += 3;
#endif

  BLTCLIP(dstLeft, dstTop, clip1min);
  BLTCLIP(dstRight, dstBottom, clip1max);

  BLTFMT(dstPitch, dstPixelFormat, bltDstFormat);
  BLTSIZE(dstWidth, dstHeight, bltDstSize);
  BLTXY(dstLeft, dstTop, bltDstXY);
  bltRop = (SSTG_ROP_SRC << 16 )| (SSTG_ROP_SRC << 8 ) | (SSTG_ROP_SRC );

#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
  dwFillColor = pbd->bltFX.dwFillColor;
  if ((DDBLT_DEPTHFILL & pbd->dwFlags) &&
      (32 == pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwZBufferBitDepth) &&
      (pbd->lpDDDestSurface->lpGbl->dwReserved1) &&
      (FX_ORIGINALLY_Z16 & ((FXSURFACEDATA *)pbd->lpDDDestSurface->lpGbl->dwReserved1)->dwSurfaceFlags))
  {
    // we should probably convert a 565 value to an 888 value
    // but to pass the Blt-Exotic 3D Depth Fill test, we need to replicate the 565 value into the
    // high word of the fill color
    dwFillColor = (dwFillColor << 16) | dwFillColor;
  }
  else if ((DDBLT_DEPTHFILL & pbd->dwFlags) &&
      (16 == pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwZBufferBitDepth) &&
      (pbd->lpDDDestSurface->lpGbl->dwReserved1) &&
      (FX_ORIGINALLY_Z32 & ((FXSURFACEDATA *)pbd->lpDDDestSurface->lpGbl->dwReserved1)->dwSurfaceFlags))
  {
    // to pass the Blt-Exotic 3D Depth Fill test, we need to do a 32bpp color fill here
    BLTFMT(dstPitch,SSTG_PIXFMT_32BPP,bltDstFormat);
  }
#endif

    // write to hw
  packetHeader |=  dstBaseAddrBit
                  | dstFormatBit
                  | ropBit
                  | clip1minBit
                  | clip1maxBit
                  | colorForeBit
                  | dstSizeBit
                  | dstXYBit
                  | commandBit;

  CMDFIFO_CHECKROOM(hwPtr, bumpNum);

  SETPH(hwPtr, CMDFIFO_BUILD_PK2( packetHeader ) );
  SETPD(hwPtr, ghw2D->dstBaseAddr,  bltDstBaseAddr);
  SETPD(hwPtr, ghw2D->dstFormat,    bltDstFormat);
  SETPD(hwPtr, ghw2D->rop,          bltRop );
  SETPD(hwPtr, ghw2D->clip1min,     clip1min);
  SETPD(hwPtr, ghw2D->clip1max,     clip1max);
  // DDraw saves the depth & color info in the same place (grab it)
#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
  SETPD(hwPtr, ghw2D->colorFore,    dwFillColor );
#else
  SETPD(hwPtr, ghw2D->colorFore,    pbd->bltFX.dwFillColor );
#endif
  SETPD(hwPtr, ghw2D->dstSize,      bltDstSize);
  SETPD(hwPtr, ghw2D->dstXY,        bltDstXY);
  SETPD(hwPtr, ghw2D->command,      SSTG_RECTFILL | SSTG_GO
    | (SSTG_ROP_SRC << SSTG_ROP0_SHIFT) | SSTG_CLIPSELECT);

#if defined(WINNT) && (_WIN32_WINNT < 0x0500)
  // restore nt invariant regs
  SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstBaseAddrBit |
                                 dstFormatBit));
  SETPD(hwPtr, ghw2D->dstBaseAddr, ppdev->ulScreenOffset);
  SETPD(hwPtr, ghw2D->dstFormat, ppdev->ulScreenFormat);
#endif

  BUMP(bumpNum)

  CMDFIFO_EPILOG(hwPtr);
  pbd->ddRVal = DD_OK;
  DDPRINT(DDDBGLVL, "<< Blt32_ColorFill (retval = %08lXh)", pbd->ddRVal);
  return DDHAL_DRIVER_HANDLED;
}// Blt32_ColorFill


/*----------------------------------------------------------------------
Function name:  Blt32_DoBltNoSP

Description:    Handle blts that don't use a source or pattern.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
Blt32_DoBltNoSP(NT9XDEVICEDATA            *ppdev,
                LPDDHAL_BLTDATA pbd,
                DWORD           rop3,
                DWORD           dstPixelFormat)
{
  DWORD bltDstBaseAddr, bltDstFormat, bltDstSize, bltDstXY ;
  DWORD dstColorkeyMax, dstColorkeyMin, dstPitch;
  DWORD bltRop, dwBltFlags, bltCommand = 0, bltCommandExtra;
  RECTL dstRect = pbd->rDest;
  long  dstLeft, dstTop, dstRight, dstBottom, dstWidth, dstHeight;
  DWORD packetHeader = 0, bumpNum = 9;
  DWORD clip1min, clip1max;

  CMDFIFO_PROLOG(hwPtr);

  // chroma key
  dwBltFlags = pbd->dwFlags;
  bltCommandExtra = 0;

  // No Src, so no source color key.

  if (dwBltFlags & DDBLT_KEYDESTOVERRIDE)
  {
    dstColorkeyMin = pbd->bltFX.ddckDestColorkey.dwColorSpaceLowValue;
    dstColorkeyMax = pbd->bltFX.ddckDestColorkey.dwColorSpaceHighValue;
    bltCommandExtra |= SSTG_EN_DST_COLORKEY_EX;
    bltCommand = SSTG_RECTFILL | SSTG_GO | (SSTG_ROP_DST << SSTG_ROP0_SHIFT)  | SSTG_CLIPSELECT;
    packetHeader |=   dstColorkeyMinBit
                    | dstColorkeyMaxBit
                    | commandExBit;
    bumpNum += 3;

  }
  else
  {
    bltCommand = SSTG_RECTFILL | SSTG_GO | (rop3 << SSTG_ROP0_SHIFT) | SSTG_CLIPSELECT;
  }

  // get rectangle
  dstTop = dstRect.top;
  dstRight = dstRect.right;
  dstBottom = dstRect.bottom;
  dstLeft = dstRect.left;

  dstWidth = dstRight - dstLeft;
  dstHeight = dstBottom - dstTop;

  // get base address
  bltDstBaseAddr = GET_HW_ADDR(pbd->lpDDDestSurface);
#if ENABLE_TILED_HEAP
  if(IS_TILED(bltDstBaseAddr) )
  {
    dstPitch = _DS(ddTileStride);
  }
  else
#endif
  {
    dstPitch = pbd->lpDDDestSurface->lpGbl->lPitch;
  }

#if defined(WINNT)
#if (_WIN32_WINNT >= 0x0500)
  // make space to restore nt invariant regs
  bumpNum += 2;
#else
  // make space to restore nt invariant regs
  bumpNum += 3;
#endif
#endif

  // now stuff them in the hardware format
  BLTCLIP(dstLeft, dstTop, clip1min);
  BLTCLIP(dstRight, dstBottom, clip1max);
  BLTFMT(dstPitch, dstPixelFormat, bltDstFormat);
  BLTSIZE(dstWidth, dstHeight, bltDstSize);
  BLTXY(dstLeft, dstTop, bltDstXY);

  bltRop = (SSTG_ROP_DST << 16 )| (SSTG_ROP_DST << 8 ) | (rop3);

  // write to hw
  packetHeader |=  dstBaseAddrBit
                  | dstFormatBit
                  | ropBit
                  | clip1minBit
                  | clip1maxBit
                  | dstSizeBit
                  | dstXYBit
                  | commandBit;

  CMDFIFO_CHECKROOM(hwPtr, bumpNum);

  SETPH( hwPtr, CMDFIFO_BUILD_PK2( packetHeader ) );
  SETPD(hwPtr, ghw2D->dstBaseAddr,    bltDstBaseAddr);
  SETPD(hwPtr, ghw2D->dstFormat,      bltDstFormat);
  if( dwBltFlags & DDBLT_KEYDESTOVERRIDE )
  {
    SETPD(hwPtr, ghw2D->dstColorkeyMin, dstColorkeyMin );
    SETPD(hwPtr, ghw2D->dstColorkeyMax, dstColorkeyMax );
  }
  SETPD(hwPtr, ghw2D->rop,            bltRop );
  if( dwBltFlags & DDBLT_KEYDESTOVERRIDE )
  {
    SETPD(hwPtr, ghw2D->commandEx,    bltCommandExtra);
  }
  SETPD(hwPtr, ghw2D->clip1min,       clip1min);
  SETPD(hwPtr, ghw2D->clip1max,       clip1max);
  SETPD(hwPtr, ghw2D->dstSize,        bltDstSize);
  SETPD(hwPtr, ghw2D->dstXY,          bltDstXY);
  SETPD(hwPtr, ghw2D->command,        bltCommand);

#if defined(WINNT)
#if (_WIN32_WINNT >= 0x0500)
  // restore nt invariant regs
  bltCommandExtra &= ~(SSTG_EN_SRC_COLORKEY_EX | SSTG_EN_DST_COLORKEY_EX);
  CMDFIFO_CHECKROOM(hwPtr, 2);
  SETPH(hwPtr, CMDFIFO_BUILD_PK2(commandExBit));
  SETPD(hwPtr, ghw2D->commandEx, bltCommandExtra);
#else
  // restore nt invariant regs
  SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstBaseAddrBit |
                                 dstFormatBit));
  SETPD(hwPtr, ghw2D->dstBaseAddr, ppdev->ulScreenOffset);
  SETPD(hwPtr, ghw2D->dstFormat, ppdev->ulScreenFormat);
#endif
#endif

  BUMP(bumpNum);
  CMDFIFO_EPILOG(hwPtr);

  pbd->ddRVal = DD_OK;
  DDPRINT(DDDBGLVL, "<< Blt32_DoBltNoSP (retval = %08lXh)", pbd->ddRVal);
  return DDHAL_DRIVER_HANDLED;
}// Blt32_DoBltNoSP


/*----------------------------------------------------------------------
Function name:  Blt32_DoBltS

Description:    Handle blts that use a source only, no pattern.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
Blt32_DoBltS(NT9XDEVICEDATA             *ppdev,
             LPDDHAL_BLTDATA  pbd,
             DWORD            rop3,
             DWORD            srcPixelFormat,
             DWORD            dstPixelFormat)
{
  DWORD bltDstBaseAddr, bltDstFormat, bltDstSize, bltDstXY ;
  DWORD bltBufBaseAddr;
  DWORD srcColorkeyMin, srcColorkeyMax, dstColorkeyMax, dstColorkeyMin;
  DWORD bltRop, dwBltFlags, bltCommand, bltCommandExtra, tmpBltCommandExtra;
  DWORD bltSrcBaseAddr, bltSrcFormat, bltSrcSize, bltSrcXY;

  DWORD dstLeft, dstTop, dstRight, dstBottom, dstWidth, dstHeight, dstPitch;
  DWORD srcLeft, srcTop, srcRight, srcBottom, srcWidth, srcHeight, srcPitch;
  DWORD srcX, srcY, dstX, dstY, clip1min, clip1max;
  DWORD packetHeader = 0;
  DWORD packetHeaderBtoS;
  DWORD bumpNum = 14;
  DWORD bumpNumBtoS;
  DWORD i;
  BOOL  Stretch;

  CMDFIFO_PROLOG(hwPtr);

  dwBltFlags = pbd->dwFlags;

  bltCommand = (DWORD)rop3 << SSTG_ROP0_SHIFT;
  bltCommandExtra = 0;

  // chroma key
  if (dwBltFlags & DDBLT_KEYSRCOVERRIDE)
  {
    srcColorkeyMin   = pbd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
    srcColorkeyMax   = pbd->bltFX.ddckSrcColorkey.dwColorSpaceHighValue;
    bltCommandExtra |= SSTG_EN_SRC_COLORKEY_EX ;
    packetHeader    |= srcColorkeyMinBit
                    |  srcColorkeyMaxBit;
    bumpNum += 2;

  }

  if (dwBltFlags & DDBLT_KEYDESTOVERRIDE)
  {
    dstColorkeyMin   = pbd->bltFX.ddckDestColorkey.dwColorSpaceLowValue;
    dstColorkeyMax   = pbd->bltFX.ddckDestColorkey.dwColorSpaceHighValue;
    bltCommandExtra |= SSTG_EN_DST_COLORKEY_EX;
    packetHeader    |= dstColorkeyMinBit
                    |  dstColorkeyMaxBit;
    bumpNum += 2;
    bltCommand = SSTG_ROP_DST << SSTG_ROP0_SHIFT;
  }

  // get rectangle
  dstTop = pbd->rDest.top;
  dstRight = pbd->rDest.right;
  dstBottom = pbd->rDest.bottom;
  dstLeft = pbd->rDest.left;

  srcTop = pbd->rSrc.top;
  srcRight = pbd->rSrc.right;
  srcBottom = pbd->rSrc.bottom;
  srcLeft = pbd->rSrc.left;

  dstWidth = dstRight - dstLeft;
  dstHeight = dstBottom - dstTop;

  srcWidth = srcRight - srcLeft;
  srcHeight = srcBottom - srcTop;

  // get base address
  bltDstBaseAddr = GET_HW_ADDR(pbd->lpDDDestSurface);
  bltSrcBaseAddr = GET_HW_ADDR(pbd->lpDDSrcSurface);
#if ENABLE_TILED_HEAP
  if(IS_TILED(bltDstBaseAddr))
  {
    dstPitch = _DS(ddTileStride);
  }
  else
#endif
  {
    dstPitch = pbd->lpDDDestSurface->lpGbl->lPitch;
  }
#if ENABLE_TILED_HEAP
  if(IS_TILED(bltSrcBaseAddr))
  {
    srcPitch = _DS(ddTileStride);
  }
  else
#endif
  {
    srcPitch = pbd->lpDDSrcSurface->lpGbl->lPitch;
  }

#if defined(WINNT)
#if (_WIN32_WINNT >= 0x0500)
  // make space to restore nt invariant regs
  bumpNum += 2;
#else
  // make space to restore nt invariant regs
  bumpNum += 6;
#endif
#endif

  // Determine if this is a stretchblt

  if ((dstWidth != srcWidth) || (dstHeight != srcHeight)) {  // STRETCH/SHRINK
      bltCommand |= SSTG_STRETCH_BLT | SSTG_GO | SSTG_CLIPSELECT;
      Stretch     = TRUE;
  } else {
      bltCommand |= SSTG_BLT         | SSTG_GO | SSTG_CLIPSELECT;
      Stretch     = FALSE;
  }

  //if destination is above and left of src, we starts srccopy at the upper left corner
  // otherwise:
  srcX = srcLeft;
  srcY = srcTop;
  dstX = dstLeft;
  dstY = dstTop;

  // Remember, we have to special-case chromakey also -- the following code
  // will NOT work for chromakey.

  if(bltDstBaseAddr == bltSrcBaseAddr)
  {
    if (!Stretch)
    {
      //IGX - ppc:Raid #318750
      if( (dstTop > srcTop) && (dstTop <= srcBottom))
      {
        // start from the bottom side
        srcY = srcBottom - 1;
        dstY = dstBottom - 1;
        bltCommand |= SSTG_YDIR;
      }
      //IGX - ppc: Raid #318750
      else if ((dstTop == srcTop) && (srcLeft<dstLeft) && (srcRight > dstLeft))
      {
        // Starts from the right side -- Reverse X Direction Blt

        // Check if chromakey is used.  If so, use special case code.
        if (!(bltCommandExtra &
             (SSTG_EN_SRC_COLORKEY_EX | SSTG_EN_DST_COLORKEY_EX))
           )
        {
          dstX = dstRight - 1;
          srcX = srcRight - 1;
          bltCommand |= SSTG_XDIR;
        }
        else
        {
          // This is not a 'Pure' bitblt.  HW doesn't directly support
          // right-to-left chromakey, stretch, or conversion blits.  For
          // chromakey, we need to so something special.  We'll blit
          // each scanline of the source to the 2D stretchbuffer, and then
          // blit it back to its actual destination using chromakey if
          // necessary.

          bltBufBaseAddr = _DS(stretchBltStart);

          // Here, set up stuff that doesn't change in Blits...
          BLTFMT(dstPitch, srcPixelFormat, bltDstFormat);
          BLTFMT(srcPitch, srcPixelFormat, bltSrcFormat);
          BLTSIZE(srcWidth, 0x00001L, bltDstSize);
          BLTCLIP(dstRight, dstBottom, clip1max);

          CMDFIFO_CHECKROOM(hwPtr, 6);
          SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstFormatBit |
                                         clip1minBit  |
                                         clip1maxBit  |
                                         srcFormatBit |
                                         dstSizeBit    ));
          SETPD(hwPtr, ghw2D->dstFormat, bltDstFormat);
          SETPD(hwPtr, ghw2D->clip1min,  0);
          SETPD(hwPtr, ghw2D->clip1max,  clip1max);
          SETPD(hwPtr, ghw2D->srcFormat, bltSrcFormat);
          SETPD(hwPtr, ghw2D->dstSize,   bltDstSize);
          BUMP(6);

          bltCommand = (rop3 << SSTG_ROP0_SHIFT) |
                       SSTG_BLT                  |
                       SSTG_CLIPSELECT           |
                       SSTG_GO;

          // Set up packetheader for screentobuffer blit
          bumpNum = 7;
          packetHeader = dstBaseAddrBit
                       | srcBaseAddrBit
                       | commandExBit
                       | srcXYBit
                       | dstXYBit
                       | commandBit;


          // Set up packetheader for buffertoscreen blit
          bumpNumBtoS = bumpNum;
          packetHeaderBtoS = packetHeader;

          if (dwBltFlags & DDBLT_KEYSRCOVERRIDE) {
            bumpNumBtoS += 1;
            packetHeaderBtoS |= ropBit;

            CMDFIFO_CHECKROOM(hwPtr, 3);
            SETPH(hwPtr, CMDFIFO_BUILD_PK2(srcColorkeyMinBit |
                                           srcColorkeyMaxBit  ));
            SETPD(hwPtr, ghw2D->srcColorkeyMin, srcColorkeyMin );
            SETPD(hwPtr, ghw2D->srcColorkeyMax, srcColorkeyMax );
            BUMP(3);

          }
          if (dwBltFlags & DDBLT_KEYDESTOVERRIDE) {
            bumpNumBtoS += 1;
            packetHeaderBtoS |= ropBit;

            CMDFIFO_CHECKROOM(hwPtr, 3);
            SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstColorkeyMinBit |
                                           dstColorkeyMaxBit  ));
            SETPD(hwPtr, ghw2D->dstColorkeyMin, dstColorkeyMin );
            SETPD(hwPtr, ghw2D->dstColorkeyMax, dstColorkeyMax );
            BUMP(3);
          }

          if (bltCommandExtra &
              (SSTG_EN_SRC_COLORKEY_EX | SSTG_EN_DST_COLORKEY_EX))
          {
            bltRop = (SSTG_ROP_DST << 16 )| (SSTG_ROP_DST << 8 ) | rop3;
          }

          // turn of colorkey before writing to stretch buffer
          tmpBltCommandExtra = bltCommandExtra &
              ~(SSTG_EN_SRC_COLORKEY_EX | SSTG_EN_DST_COLORKEY_EX);

          // Start of scanline blit loop
          for (i=srcTop; i <= srcBottom; i++) {

            // Move scan from src to Stretchbuffer
            BLTXY(0x0L,    0x0L, bltDstXY);
            BLTXY(srcLeft, i,    bltSrcXY);
            CMDFIFO_CHECKROOM(hwPtr, bumpNum);
            SETPH(hwPtr, CMDFIFO_BUILD_PK2(packetHeader));
            SETPD(hwPtr, ghw2D->dstBaseAddr, bltBufBaseAddr);
            SETPD(hwPtr, ghw2D->srcBaseAddr, bltSrcBaseAddr);
            SETPD(hwPtr, ghw2D->commandEx,   tmpBltCommandExtra);
            SETPD(hwPtr, ghw2D->srcXY,       bltSrcXY);
            SETPD(hwPtr, ghw2D->dstXY,       bltDstXY);
            SETPD(hwPtr, ghw2D->command,     bltCommand);

            // Move scan from stretchbuffer to dest, using ColorKeys if needed
            BLTXY(dstLeft, i,    bltDstXY);
            BLTXY(0x0L,    0x0L, bltSrcXY);
            CMDFIFO_CHECKROOM(hwPtr, bumpNumBtoS);
            SETPH(hwPtr, CMDFIFO_BUILD_PK2(packetHeaderBtoS));
            SETPD(hwPtr, ghw2D->dstBaseAddr, bltDstBaseAddr);
            if (bltCommandExtra &
              (SSTG_EN_SRC_COLORKEY_EX | SSTG_EN_DST_COLORKEY_EX))
            {
              SETPD(hwPtr, ghw2D->rop, bltRop);
            }
            SETPD(hwPtr, ghw2D->srcBaseAddr, bltBufBaseAddr);
            SETPD(hwPtr, ghw2D->commandEx,   bltCommandExtra);
            SETPD(hwPtr, ghw2D->srcXY,       bltSrcXY);
            SETPD(hwPtr, ghw2D->dstXY,       bltDstXY);
            SETPD(hwPtr, ghw2D->command,     bltCommand);

          }

          goto B32DBS_Exit;     //  We're all done now

        }
      }
    }
    // STRETCH BLT:   (What to do with Shrinks?  we should fail them.
    else if (!(srcRight < dstLeft) || (srcLeft > dstRight))  // Overlap in X?
    {
      // Now check for COMPLETE overlap in Y
      if ( ( (srcTop    > dstTop) && (srcTop    < dstBottom) )  &&
           ( (srcBottom > dstTop) && (srcBottom < dstBottom) ) )
      {
        // Do special Move Blit -- As Russ Lind suggested, in this case we
        // blit the original bitmap to the lower-right corner of the dest
        // rectangle, and then do the stretch from there.  The first blit
        // is straight screen-to-screen srccopy

        BLTCLIP(dstRight - srcWidth, dstBottom - srcHeight, clip1min);
        BLTCLIP(dstRight,            dstBottom,             clip1max);

        // Format for HW
        BLTFMT(dstPitch, srcPixelFormat, bltDstFormat);
        BLTFMT(srcPitch, srcPixelFormat, bltSrcFormat);

        BLTSIZE(srcWidth, srcHeight, bltDstSize);

        // start from the bottom side
        srcY = srcBottom - 1;
        dstY = dstBottom - 1;
        dstX = dstRight  - 1;
        srcX = srcRight  - 1;

        BLTXY(dstX, dstY, bltDstXY);
        BLTXY(srcX, srcY, bltSrcXY);

        // Write to hw
        packetHeader |= dstBaseAddrBit
                     |  dstFormatBit
                     |  srcBaseAddrBit
                     |  clip1minBit
                     |  clip1maxBit
                     |  srcFormatBit
                     |  srcXYBit
                     |  dstSizeBit
                     |  dstXYBit
                     |  commandBit;

        CMDFIFO_CHECKROOM(hwPtr, 11);

        SETPH(hwPtr, CMDFIFO_BUILD_PK2(packetHeader));

        SETPD(hwPtr, ghw2D->dstBaseAddr, bltDstBaseAddr);
        SETPD(hwPtr, ghw2D->dstFormat,   bltDstFormat);
        SETPD(hwPtr, ghw2D->srcBaseAddr, bltSrcBaseAddr);
        SETPD(hwPtr, ghw2D->clip1min,    clip1min);
        SETPD(hwPtr, ghw2D->clip1max,    clip1max);
        SETPD(hwPtr, ghw2D->srcFormat,   bltSrcFormat);
        SETPD(hwPtr, ghw2D->srcXY,       bltSrcXY);
        SETPD(hwPtr, ghw2D->dstSize,     bltDstSize);
        SETPD(hwPtr, ghw2D->dstXY,       bltDstXY);
        SETPD(hwPtr, ghw2D->command,    (DWORD)(SSTG_ROP_SRC << SSTG_ROP0_SHIFT)
                                              | SSTG_BLT
                                              | SSTG_GO
                                              | SSTG_CLIPSELECT
                                              | SSTG_YDIR
                                              | SSTG_XDIR);

        // Just fake out locals to use the rest of the HW code below...
        dstX = dstLeft;
        dstY = dstTop;
        srcX = dstRight  - srcWidth;
        srcY = dstBottom - srcHeight;

      }
      // If we overlap bottom only, do reverse Y blt.  If overlap in Top
      // only then we do nothing special -- normal top-down blit.
      else if (srcBottom > dstTop && srcBottom < dstBottom) // Bottom Only?
      {
        // Do Reverse Y Blt
        srcY = srcBottom - 1;
        dstY = dstBottom -1 ;
        bltCommand |= SSTG_YDIR;
      }
    }
  }

  BLTCLIP(dstLeft, dstTop, clip1min);
  BLTCLIP(dstRight, dstBottom, clip1max);

  // now stuff them in the hardware format
  BLTFMT(dstPitch, dstPixelFormat, bltDstFormat);
  BLTSIZE(dstWidth, dstHeight, bltDstSize);
  BLTXY(dstX, dstY, bltDstXY);

  BLTFMT(srcPitch, srcPixelFormat, bltSrcFormat);  // 16 bpp for now
  BLTSIZE(srcWidth, srcHeight, bltSrcSize);
  BLTXY(srcX, srcY, bltSrcXY);

  bltRop = (SSTG_ROP_DST << 16 )| (SSTG_ROP_DST << 8 ) | rop3;


  // write to hw
  packetHeader |= dstBaseAddrBit
                  | dstFormatBit
                  | ropBit
                  | srcBaseAddrBit
                  | commandExBit
                  | clip1minBit
                  | clip1maxBit
                  | srcFormatBit
                  | srcSizeBit
                  | srcXYBit
                  | dstSizeBit
                  | dstXYBit
                  | commandBit;

        CMDFIFO_CHECKROOM(hwPtr, bumpNum);

        SETPH( hwPtr, CMDFIFO_BUILD_PK2( packetHeader ) );
        SETPD(hwPtr, ghw2D->dstBaseAddr, bltDstBaseAddr);
        SETPD(hwPtr, ghw2D->dstFormat, bltDstFormat);
        if(dwBltFlags & DDBLT_KEYSRCOVERRIDE)
        {
                SETPD( hwPtr, ghw2D->srcColorkeyMin, srcColorkeyMin );
                SETPD( hwPtr, ghw2D->srcColorkeyMax, srcColorkeyMax );
        }
        if(dwBltFlags & DDBLT_KEYDESTOVERRIDE)
        {
                SETPD( hwPtr, ghw2D->dstColorkeyMin, dstColorkeyMin );
                SETPD( hwPtr, ghw2D->dstColorkeyMax, dstColorkeyMax );
        }
        SETPD(hwPtr, ghw2D->rop, bltRop );
        SETPD(hwPtr, ghw2D->srcBaseAddr, bltSrcBaseAddr);
        SETPD(hwPtr, ghw2D->commandEx, bltCommandExtra);
        SETPD(hwPtr, ghw2D->clip1min, clip1min);
        SETPD(hwPtr, ghw2D->clip1max, clip1max);
        SETPD(hwPtr, ghw2D->srcFormat, bltSrcFormat);
        SETPD(hwPtr, ghw2D->srcSize, bltSrcSize);
        SETPD(hwPtr, ghw2D->srcXY,bltSrcXY);
        SETPD(hwPtr, ghw2D->dstSize, bltDstSize);
        SETPD(hwPtr, ghw2D->dstXY,bltDstXY);
        SETPD(hwPtr, ghw2D->command, bltCommand);

#if defined(WINNT)
#if (_WIN32_WINNT >= 0x0500)
  bltCommandExtra &= ~(SSTG_EN_SRC_COLORKEY_EX | SSTG_EN_DST_COLORKEY_EX);
  SETPH(hwPtr, CMDFIFO_BUILD_PK2(commandExBit));
  SETPD(hwPtr, ghw2D->commandEx, bltCommandExtra);
#else
        // restore nt invariant regs
        SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstBaseAddrBit |
                                                                                                                                 dstFormatBit   |
                                                                                                                                 srcBaseAddrBit |
                                                                                                                                 commandExBit   |
                                                                                                                                 srcFormatBit));
        SETPD(hwPtr, ghw2D->dstBaseAddr, ppdev->ulScreenOffset);
        SETPD(hwPtr, ghw2D->dstFormat, ppdev->ulScreenFormat);
        SETPD(hwPtr, ghw2D->srcBaseAddr, ppdev->ulScreenOffset);
        SETPD(hwPtr, ghw2D->commandEx, 0);
        SETPD(hwPtr, ghw2D->srcFormat, ppdev->ulScreenFormat);
#endif
#endif

        BUMP(bumpNum)

B32DBS_Exit:

        CMDFIFO_EPILOG(hwPtr);


  pbd->ddRVal = DD_OK;
  DDPRINT(DDDBGLVL, "<< Blt32_DoBltS (retval = %08lXh)", pbd->ddRVal);
  return DDHAL_DRIVER_HANDLED;
}// Blt32_DoBltS


/*----------------------------------------------------------------------
Function name:  Blt32_SystemToVideo

Description:

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
Blt32_SystemToVideo(NT9XDEVICEDATA  *ppdev,
                    LPDDHAL_BLTDATA pbd,
                    DWORD           rop3,
                    DWORD           srcPixelFormat,
                    DWORD           srcBytePerPixel,
                    DWORD           dstPixelFormat)
{
  DWORD           bltDstBaseAddr, bltDstFormat, bltDstSize, bltDstXY, bltSrcFormat;
  DWORD           srcColorkeyMin, srcColorkeyMax, dstColorkeyMax, dstColorkeyMin;
  DWORD           bltRop, dwBltFlags, bltCommand = 0, bltCommandExtra;
  DWORD           packetHeader = 0, bumpNum = 13, clip1min, clip1max;
  DWORD           bltSrcSize, bytesRemaining;
  long            dstLeft, dstTop, dstWidth, dstHeight, dstPitch;
  long            srcWidth, srcHeight, srcPitch;
  long            i, j, *scaneline, num_dwWrite, num_full_dwWrite, dwPartial;
  int             errorterm;
  int             twodeltaX;
  int             twodeltaY;
  char            *srcOffset, *src, *srcTmp;

  CMDFIFO_PROLOG(hwPtr);

#ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "Handle CPU To Screen" ));
#endif
  dwBltFlags = pbd->dwFlags;
  bltCommandExtra = 0;

  if (dwBltFlags & DDBLT_KEYSRCOVERRIDE)
  {
    srcColorkeyMin = pbd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
    srcColorkeyMax = pbd->bltFX.ddckSrcColorkey.dwColorSpaceHighValue;
    bltCommandExtra |= SSTG_EN_SRC_COLORKEY_EX;
    packetHeader |=   srcColorkeyMinBit
                    | srcColorkeyMaxBit;
    bumpNum += 2;
  }

  if (dwBltFlags & DDBLT_KEYDESTOVERRIDE)
  {
    dstColorkeyMin = pbd->bltFX.ddckDestColorkey.dwColorSpaceLowValue;
    dstColorkeyMax = pbd->bltFX.ddckDestColorkey.dwColorSpaceHighValue;
    bltCommandExtra |= SSTG_EN_DST_COLORKEY_EX;
    packetHeader |=   dstColorkeyMinBit
                    | dstColorkeyMaxBit;
    bumpNum += 2;
    bltCommand |= SSTG_ROP_DST << SSTG_ROP0_SHIFT;  // ROP0 = dst copy
    bltRop = rop3;                                  // ROP1 = rop3
  }
  else
  {
    bltCommand |= rop3 << SSTG_ROP0_SHIFT;          // ROP0 = rop3
    bltRop = SSTG_ROP_DST << 8;                     // ROP2 = dst copy
  }
  // The case with both src colorkey & dst colorkey used simultaneously
  // is not being handled.  What would be correct for that case

  bltDstBaseAddr = GET_HW_ADDR(pbd->lpDDDestSurface);
#if ENABLE_TILED_HEAP
  if(IS_TILED(bltDstBaseAddr))
  {
    dstPitch = _DS(ddTileStride);
  }
  else
#endif
  {
    dstPitch = pbd->lpDDDestSurface->lpGbl->lPitch;
  }

  // get rectangle
  dstTop = pbd->rDest.top;
  dstLeft = pbd->rDest.left;

  dstWidth = pbd->rDest.right - dstLeft;  // for now no stretch blit - SS
  dstHeight = pbd->rDest.bottom - dstTop;

  srcOffset = (char*) pbd->lpDDSrcSurface->lpGbl->fpVidMem;   // in byte
  srcPitch  = pbd->lpDDSrcSurface->lpGbl->lPitch;             // in byte

  srcWidth = pbd->rSrc.right - pbd->rSrc.left;
  srcHeight = pbd->rSrc.bottom - pbd->rSrc.top;

  src = (char*) ( srcOffset + ( pbd->rSrc.top * srcPitch)
    + (pbd->rSrc.left * srcBytePerPixel ) ) ;

  // prepare register values
  BLTCLIP(dstLeft, dstTop, clip1min);
  BLTCLIP(pbd->rDest.right, pbd->rDest.bottom, clip1max);
  BLTFMT(dstPitch, dstPixelFormat, bltDstFormat);  // 16 bpp for now
  BLTSIZE(srcWidth, srcHeight, bltSrcSize);
  BLTSIZE(dstWidth, dstHeight, bltDstSize);
  BLTXY(dstLeft, dstTop, bltDstXY);

  BLTFMT(srcPitch, srcPixelFormat, bltSrcFormat);

  if ((dstWidth != srcWidth) || (dstHeight != srcHeight))
    bltCommand |= SSTG_HOST_STRETCH_BLT | SSTG_CLIPSELECT;
  else
    bltCommand |= SSTG_HOST_BLT | SSTG_CLIPSELECT;

  // write to hw
  packetHeader |=  dstBaseAddrBit
                  | dstFormatBit
                  | ropBit
                  | commandExBit
                  | clip1minBit
                  | clip1maxBit
                  | srcFormatBit
                  | srcSizeBit
                  | srcXYBit
                  | dstSizeBit
                  | dstXYBit
                  | commandBit;

  CMDFIFO_CHECKROOM(hwPtr, bumpNum);

  SETPH(hwPtr, CMDFIFO_BUILD_PK2( packetHeader ) );
  SETPD(hwPtr, ghw2D->dstBaseAddr,        bltDstBaseAddr);
  SETPD(hwPtr, ghw2D->dstFormat,          bltDstFormat);
  if(dwBltFlags & DDBLT_KEYSRCOVERRIDE)
  {
    SETPD( hwPtr, ghw2D->srcColorkeyMin,  srcColorkeyMin );
    SETPD( hwPtr, ghw2D->srcColorkeyMax,  srcColorkeyMax );
  }
  if(dwBltFlags & DDBLT_KEYDESTOVERRIDE)
  {
    SETPD( hwPtr, ghw2D->dstColorkeyMin,    dstColorkeyMin );
    SETPD( hwPtr, ghw2D->dstColorkeyMax,    dstColorkeyMax );
  }
  SETPD( hwPtr, ghw2D->rop,               bltRop );
  SETPD( hwPtr, ghw2D->commandEx,         bltCommandExtra);
  SETPD( hwPtr, ghw2D->clip1min,          clip1min);
  SETPD( hwPtr, ghw2D->clip1max,          clip1max);
  SETPD( hwPtr, ghw2D->srcFormat,         bltSrcFormat);
  SETPD( hwPtr, ghw2D->srcSize,           bltSrcSize);
  SETPD( hwPtr, ghw2D->srcXY,             0);
  SETPD( hwPtr, ghw2D->dstSize,           bltDstSize);
  SETPD( hwPtr, ghw2D->dstXY,             bltDstXY);
  SETPD( hwPtr, ghw2D->command,           bltCommand );

  BUMP(bumpNum);

  // Compute the number of dwords to write per scanline
  // If there is a partial dword at the tail of the scanline
  // round the number of dwords up
  num_dwWrite = (srcWidth * srcBytePerPixel + 3) / 4;
  num_full_dwWrite = (srcWidth * srcBytePerPixel) / 4;
  bytesRemaining   = (srcWidth * srcBytePerPixel) & 3;

  if (SSTG_HOST_BLT & bltCommand)  // Non-Stretch Blit
  {
    for( i = 0; i < srcHeight; i++)
    {
      scaneline = (DWORD*)src;
      CMDFIFO_CHECKROOM( hwPtr, (unsigned long)( num_dwWrite + 1 ) );
      // we should probably write these into the launch area 32 dwords
      // at a time, rather than one dword at a time
      SETPH( hwPtr, CMDFIFO_BUILD_2DPK1( num_dwWrite , 0, launch[0], 0xF ) );
      for(j = 0; j < num_full_dwWrite; j++)
      {
        SETPD( hwPtr, ghw2D->launch[0], *scaneline );
        scaneline++;
      }
      if (bytesRemaining)
      {
        srcTmp    = (UCHAR*)scaneline;
        dwPartial = 0;
        if (bytesRemaining == 1)
        {
          dwPartial = (DWORD) ((UCHAR) *srcTmp);
        }
        else if (bytesRemaining == 2)
        {
          dwPartial  = (DWORD) ((UCHAR) *srcTmp++);
          dwPartial |= (DWORD) ((UCHAR) *srcTmp) << 8;
        }
        else // 3
        {
          dwPartial  = (DWORD) ((UCHAR) *srcTmp++);
          dwPartial |= (DWORD) ((UCHAR) *srcTmp++) << 8;
          dwPartial |= (DWORD) ((UCHAR) *srcTmp) << 16;
        }
        SETPD( hwPtr, ghw2D->launch[0], dwPartial );
      }
      src += srcPitch;
      BUMP((unsigned long)( num_dwWrite + 1 ))
    }
  }
  else  // Stretch Blt
  {
    //  We have to stretch in Y by replicating scanlines.  We do this using
    //  standard bresenham to tell us when it's time to move to the next
    //  source scanline.

    twodeltaX = (int)(srcHeight + srcHeight);
    twodeltaY = (int)(dstHeight + dstHeight);
    errorterm = twodeltaX + (int)srcHeight - twodeltaY;

    for (i = 0; i < dstHeight; i++)
    {
      scaneline = (DWORD*)src;

      CMDFIFO_CHECKROOM( hwPtr, (unsigned long)( num_dwWrite + 1 ) );
      // we should probably write these into the launch area 32 dwords
      // at a time, rather than one dword at a time
      SETPH(hwPtr,CMDFIFO_BUILD_2DPK1( num_dwWrite, 0, launch[0], 0xF));
      for(j = 0; j < num_full_dwWrite; j++)
      {
        SETPD( hwPtr, ghw2D->launch[0], *scaneline );
        scaneline++;
      }
      if (bytesRemaining)
      {
        srcTmp    = (UCHAR*)scaneline;
        dwPartial = 0;
        if (bytesRemaining == 1)
        {
          dwPartial = (DWORD) ((UCHAR) *srcTmp);
        }
        else if (bytesRemaining == 2)
        {
          dwPartial  = (DWORD) ((UCHAR) *srcTmp++);
          dwPartial |= (DWORD) ((UCHAR) *srcTmp) << 8;
        }
        else // 3
        {
          dwPartial  = (DWORD) ((UCHAR) *srcTmp++);
          dwPartial |= (DWORD) ((UCHAR) *srcTmp++) << 8;
          dwPartial |= (DWORD) ((UCHAR) *srcTmp) << 16;
        }
        SETPD( hwPtr, ghw2D->launch[0], dwPartial );
      }

      BUMP((unsigned long)( num_dwWrite + 1 ))


      while (errorterm >= 0)
      {
        src       += srcPitch;
        errorterm -= twodeltaY;
      }

      errorterm += twodeltaX;
    }
  }

#if defined(WINNT)
#if (_WIN32_WINNT >= 0x0500)
  // restore nt invariant regs
  bltCommandExtra &= ~(SSTG_EN_SRC_COLORKEY_EX | SSTG_EN_DST_COLORKEY_EX);
  CMDFIFO_CHECKROOM(hwPtr, 2);
  SETPH(hwPtr, CMDFIFO_BUILD_PK2(commandExBit));
  SETPD(hwPtr, ghw2D->commandEx, bltCommandExtra);
  BUMP(2);
#else
  // restore nt invariant regs
  CMDFIFO_CHECKROOM(hwPtr, 5);
  SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstBaseAddrBit |
                                 dstFormatBit   |
                                 commandExBit   |
                                 srcFormatBit));
  SETPD(hwPtr, ghw2D->dstBaseAddr, ppdev->ulScreenOffset);
  SETPD(hwPtr, ghw2D->dstFormat, ppdev->ulScreenFormat);
  SETPD(hwPtr, ghw2D->commandEx, 0);
  SETPD(hwPtr, ghw2D->srcFormat, ppdev->ulScreenFormat);
  BUMP(5);
#endif
#endif

  CMDFIFO_EPILOG( hwPtr );

  pbd->ddRVal = DD_OK;
  DDPRINT(DDDBGLVL, "<< Blt32_SystemToVideo (retval = %08lXh)", pbd->ddRVal);
  return DDHAL_DRIVER_HANDLED;
}// Blt32_SystemToVideo

/*----------------------------------------------------------------------
Function name:  Blt32_CopyFourCC()

Description:

Return:         HRESULT

From the W2K DDK "Using Compressed Texture Surfaces" section (Section 3.11.3)

DirectDraw will only call the driver to do a blit between two surfaces of
the same DXT type if the DDCAPS2_COPYFOURCC flag is set. If this flag is
not set, the HEL will perform the blit. This is most important for
system-to-display copy blits, since this is the mechanism whereby textures
are downloaded from system memory to display memory. Exposing DXT texture
surfaces thereby effectively requires your driver to support the
DDCAPS2_COPYFOURCC flag.

The DDCAPS2_COPYFOURCC flag has some additional implications.
Your driver must be able to execute a blit between FourCC formats
having at least these attributes:

  - The source and destination formats are the same FourCC.
  - The source and destination surfaces are not the same surface.
  - The source and destination surfaces are the same size.
  - The source and destination rectangles are both the entire surface
          (that is, no stretching, and no sub-rectangles).
  - Both surfaces are in display memory.
  - The driver must be able to perform these blits for every FourCC
    format it supports in display memory.

If a blit operation requires compression to a DXT format, the DirectDraw
HEL always performs the blit. This means that DirectDraw will never
request the driver to perform a blit for which:

  - The destination surface has a DXT format.
  - The formats for the source and destination surfaces are not the same.

The semantics of the DirectDraw DDCAPS_CANBLTSYSMEM capability bit imply
that the display driver be called for all system memory to display memory
blits. Consequently, the driver may be called for system memory to display
memory blits from DXT surfaces to non-DXT surfaces. The only requirement
in this case is that the driver return DDHAL_DRIVER_NOTHANDLED if it cannot
perform the decompression. This will cause DirectDraw to propagate a
DDERR_UNSUPPORTED error code to the application. It is acceptable to
implement decompression for system memory to display memory blits in your
driver, but this is not required.

Note: I've always thought passing a LPDDHAL_BLTDATA pointer to the
      blt functions in this file was a brain dead decision, so I've
      opted not to pass a LPDDHAL_BLTDATA to this function so that
      this function can be called from elsewhere.  In particular, I
      expect this function to be called from the TEXBLT handler in
      ddiDrawPrimitives2 or possibly from the TEXTURELOAD function

----------------------------------------------------------------------*/

DWORD __stdcall
Blt32_CopyFourCC(NT9XDEVICEDATA             *ppdev,
                 LPDDRAWI_DDRAWSURFACE_LCL  pDDSrcSurf,
                 RECTL                      *pSrcRect,
                 LPDDRAWI_DDRAWSURFACE_LCL  pDDDstSurf,
                 RECTL                      *pDstRect)
{
  DWORD dstBaseAddr, srcBaseAddr;
  DWORD dstPitch, srcPitch;
  DWORD bitsPerPel, width, height;
  DWORD dstFormat, srcFormat;
  DWORD dstSize;
  DWORD clip1min, clip1max;
  DWORD bumpNum = 11;
#if ENABLE_3D
  DWORD buffer[32]; // This is a work buffer for padding DXT1 textures.
                    // We know that the worst we can be is 8:1
                    // This case revolves around having a width of 4
                    // or less, therefore the tallest the texture can
                    // be is 32 pixels.  We are padding to 8xheight.
                    // At 4bpp the worst case is (8x32)/2 bytes.
                    // That happens to be 32 DWORDs.
#endif

  // check for equal FourCC's
  if (pDDSrcSurf->lpGbl->ddpfSurface.dwFourCC != pDDDstSurf->lpGbl->ddpfSurface.dwFourCC)
  {
    DDPRINT(0, "Blt32_CopyFourCC called with unequal FourCC's in dst & src");
    return DDERR_UNSUPPORTED;
  }
  // check for not same surface
  if (pDDSrcSurf == pDDDstSurf)
  {
    DDPRINT(0, "Blt32_CopyFourCC called with same surface for src & dst");
    return DDERR_UNSUPPORTED;
  }
  // check for same size src & dst
#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
  // the DX7 runtime appears to munge the width and height of system memory DXTn
  // surfaces
  if ((DDSCAPS_SYSTEMMEMORY & pDDSrcSurf->ddsCaps.dwCaps) &&
      ((FOURCC_DXT1 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
       (FOURCC_DXT2 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
       (FOURCC_DXT3 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
       (FOURCC_DXT4 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
       (FOURCC_DXT5 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC)))
  {
    DWORD dstHeightToMatch, dstWidthToMatch;
    DWORD surfSize;

    dstHeightToMatch = (pDDDstSurf->lpGbl->wHeight + 3) / 4;

    surfSize = ((pDDDstSurf->lpGbl->wWidth  + 3) / 4) * dstHeightToMatch;
    if (FOURCC_DXT1 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC)
      surfSize *= 8;    // The size of a DXT1 4x4 pixel block
    else
      surfSize *= 16;   // The size of a DXT2,DXT3,DXT4 or DXT5 4x4 pixel block

    dstWidthToMatch = surfSize / dstHeightToMatch;

    if (! ((pDDSrcSurf->lpGbl->wWidth  == dstWidthToMatch ) &&
           (pDDSrcSurf->lpGbl->wHeight == dstHeightToMatch)))
    {
      DDPRINT(0, "Blt32_CopyFourCC called with different size src & dst");
      return DDERR_UNSUPPORTED;
    }
  }
  else
#endif
  if (! ((pDDSrcSurf->lpGbl->wWidth  == pDDDstSurf->lpGbl->wWidth ) &&
         (pDDSrcSurf->lpGbl->wHeight == pDDDstSurf->lpGbl->wHeight)))
  {
    DDPRINT(0, "Blt32_CopyFourCC called with different size src & dst");
    return DDERR_UNSUPPORTED;
  }

  // check for subrectangle
  if (! ((0 == pDstRect->left) && ((DWORD)pDstRect->right  == pDDDstSurf->lpGbl->wWidth ) &&
           (0 == pDstRect->top ) && ((DWORD)pDstRect->bottom == pDDDstSurf->lpGbl->wHeight)))
  {
    DDPRINT(0, "Blt32_CopyFourCC called with subrectangle");
    return DDERR_UNSUPPORTED;
  }
  // check for dst in video memory
  // due to CANBLTSYSMEM, the src may be in system memory
  if (! (DDSCAPS_VIDEOMEMORY & pDDDstSurf->ddsCaps.dwCaps))
  {
    DDPRINT(0, "Blt32_CopyFourCC called with non video memory dst surface");
    return DDERR_UNSUPPORTED;
  }

#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
   DDPRINT(DDDBGLVL, "Blt32_CopyFourCC, pDDSrcSurf=%8lXh (hSrc = %ld) pDDDstSurf=%8lXh (hDst=%ld)",
           pDDSrcSurf, pDDSrcSurf->lpSurfMore->dwSurfaceHandle,
           pDDDstSurf, pDDDstSurf->lpSurfMore->dwSurfaceHandle);
#endif


  // src in video memory
  if (DDSCAPS_VIDEOMEMORY & pDDSrcSurf->ddsCaps.dwCaps)
  {
    CMDFIFO_PROLOG(hwPtr);

    ASSERTDD(DDSCAPS_VIDEOMEMORY & pDDSrcSurf->ddsCaps.dwCaps,
             "Blt32_CopyFourCC expected a video memory src surface");

    // get base address
#ifdef SIMULATE_YV12
    if (FOURCC_YV12 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC)
    {
      dstBaseAddr = pDDDstSurf->lpGbl->fpVidMem;
      srcBaseAddr = pDDSrcSurf->lpGbl->fpVidMem;
    }
    else
#endif
    {
      // dst is in video memory
      // check for a FXSURFACEDATA struct before dereferencing it
      if (pDDDstSurf->lpGbl->dwReserved1)
      {
        // surface has an FXSURFACEDATA struct so get the hwOffset from FXSURFACEDATA
        dstBaseAddr = GET_HW_ADDR(pDDDstSurf);
      }
      else
      {
        // surface does not have an FXSURFACEDATA struct, use fpVidMem to compute the hwOffset
        // this assumes the surface is in linear memory
#ifdef WINNT
        dstBaseAddr = pDDDstSurf->lpGbl->fpVidMem;
#else
        dstBaseAddr = pDDDstSurf->lpGbl->fpVidMem - _DS(LFBBASE);
#endif
      }
      // src is in video memory
      // check for a FXSURFACEDATA struct before dereferencing it
      if (pDDSrcSurf->lpGbl->dwReserved1)
      {
        // surface has an FXSURFACEDATA struct so get the hwOffset from FXSURFACEDATA
        srcBaseAddr = GET_HW_ADDR(pDDSrcSurf);
      }
      else
      {
        // surface does not have an FXSURFACEDATA struct, use fpVidMem to compute the hwOffset
        // this assumes the surface is in linear memory
#ifdef WINNT
        srcBaseAddr = pDDSrcSurf->lpGbl->fpVidMem;
#else
        srcBaseAddr = pDDSrcSurf->lpGbl->fpVidMem - _DS(LFBBASE);
#endif
      }
    }
#if ENABLE_TILED_HEAP
    if(IS_TILED(dstBaseAddr))
    {
      dstPitch = _DS(ddTileStride);
    }
    else
#endif
#if ENABLE_3D
    if ((FOURCC_DXT1 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
        (FOURCC_FXT1 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC))
    {
      dstPitch = (((pDstRect->right - pDstRect->left) * 4) + 7) / 8;
    }
    else if ((FOURCC_DXT2 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT3 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT4 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT5 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC))
    {
      dstPitch = (((pDstRect->right - pDstRect->left) * 8) + 7) / 8;
    }
    else
#endif
    {
      dstPitch = pDDDstSurf->lpGbl->lPitch;
    }
#if ENABLE_TILED_HEAP
    if(IS_TILED(srcBaseAddr))
    {
      srcPitch = _DS(ddTileStride);
    }
    else
#endif
#if ENABLE_3D
    if ((FOURCC_DXT1 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
        (FOURCC_FXT1 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC))
    {
      srcPitch = (((pDstRect->right - pDstRect->left) * 4) + 7) / 8;
    }
    else if ((FOURCC_DXT2 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT3 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT4 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT5 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC))
    {
      srcPitch = (((pDstRect->right - pDstRect->left) * 8) + 7) / 8;
    }
    else
#endif
    {
      srcPitch = pDDSrcSurf->lpGbl->lPitch;
    }

#if defined(WINNT) && (_WIN32_WINNT < 0x0500)
    // make space to restore nt invariant regs
    bumpNum += 5;
#endif

#if ENABLE_3D
    if ((FOURCC_DXT1 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
        (FOURCC_FXT1 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC))
      bitsPerPel = 4;
    else if ((FOURCC_DXT2 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT3 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT4 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT5 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC))
      bitsPerPel = 8;
    else
#endif
      bitsPerPel = pDDDstSurf->lpGbl->ddpfSurface.dwRGBBitCount;

    // this calculation of width and height should handle
    // UYVY, YUY2, DXTn and FXT1 surfaces
    width = (((pDstRect->right - pDstRect->left) * bitsPerPel) + 7) / 8;
    height = pDstRect->bottom - pDstRect->top;

#ifdef SIMULATE_YV12
        if (FOURCC_YV12 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC)
    {
      // YV12 is an oddball format where bitsPerPel is 12
      // but the width should be computed as if it were 8bpp
      // and the height is 3/2 the height calculated above

      ASSERTDD(FOURCC_YV12 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC,
               "Blt32_CopyFourCC called with oddball format other than YV12");

      width = pDstRect->right - pDstRect->left;
      height = (bitsPerPel * height) / 8;

      // the YV12 data is actually in system memory so we
      // don't want to do a hw blt here
      // we need to do a memcpy from the src to the dst
      // and then return

      ASSERTDD((! IS_TILED(dstBaseAddr)) && (! IS_TILED(srcBaseAddr)),
               "Blt32_CopyFourCC called with YV12 addr in tiled space?");
      memcpy((LPVOID)dstBaseAddr, (LPVOID)srcBaseAddr, width * height);
      return DD_OK;
    }
#endif

    BLTCLIP(0, 0, clip1min);
    BLTCLIP(width, height, clip1max);

    BLTFMT(dstPitch, SSTG_PIXFMT_8BPP, dstFormat);
    BLTFMT(srcPitch, SSTG_PIXFMT_8BPP, srcFormat);

    BLTSIZE(width, height, dstSize);

    CMDFIFO_CHECKROOM(hwPtr, bumpNum);

    SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstBaseAddrBit |
                                   dstFormatBit   |
                                   srcBaseAddrBit |
                                   clip1minBit    |
                                   clip1maxBit    |
                                   srcFormatBit   |
                                   srcXYBit       |
                                   dstSizeBit     |
                                   dstXYBit       |
                                   commandBit));
    SETPD(hwPtr, ghw2D->dstBaseAddr, dstBaseAddr);
    SETPD(hwPtr, ghw2D->dstFormat,   dstFormat);
    SETPD(hwPtr, ghw2D->srcBaseAddr, srcBaseAddr);
    SETPD(hwPtr, ghw2D->clip1min,    clip1min);
    SETPD(hwPtr, ghw2D->clip1max,    clip1max);
    SETPD(hwPtr, ghw2D->srcFormat,   srcFormat);
    SETPD(hwPtr, ghw2D->srcXY,       0);
    SETPD(hwPtr, ghw2D->dstSize,     dstSize);
    SETPD(hwPtr, ghw2D->dstXY,       0);
    SETPD(hwPtr, ghw2D->command,     SSTG_ROP_SRCCOPY |
                                     SSTG_BLT         |
                                     SSTG_CLIPSELECT  |
                                     SSTG_GO);

#if defined(WINNT) && (_WIN32_WINNT < 0x0500)
    // restore nt invariant regs
    SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstBaseAddrBit |
                                   dstFormatBit   |
                                   srcBaseAddrBit |
                                   srcFormatBit));
    SETPD(hwPtr, ghw2D->dstBaseAddr, ppdev->ulScreenOffset);
    SETPD(hwPtr, ghw2D->dstFormat,   ppdev->ulScreenFormat);
    SETPD(hwPtr, ghw2D->srcBaseAddr, ppdev->ulScreenOffset);
    SETPD(hwPtr, ghw2D->srcFormat,   ppdev->ulScreenFormat);
#endif

          BUMP(bumpNum);

          CMDFIFO_EPILOG(hwPtr);
  }
  // src in system memory
  else
  {
    DWORD numDwords, i, j, *scaneline;

    CMDFIFO_PROLOG(hwPtr);

    ASSERTDD(DDSCAPS_SYSTEMMEMORY & pDDSrcSurf->ddsCaps.dwCaps,
             "Blt32_CopyFourCC expected a system memory src surface");

    // get base address
    // dst is in video memory
    // check for a FXSURFACEDATA struct before dereferencing it
    if (pDDDstSurf->lpGbl->dwReserved1)
    {
      // surface has an FXSURFACEDATA struct so get the hwOffset from FXSURFACEDATA
      dstBaseAddr = GET_HW_ADDR(pDDDstSurf);
    }
    else
    {
      // surface does not have an FXSURFACEDATA struct, use fpVidMem to compute the hwOffset
      // this assumes the surface is in linear memory
#ifdef WINNT
      dstBaseAddr = pDDDstSurf->lpGbl->fpVidMem;
#else
      dstBaseAddr = pDDDstSurf->lpGbl->fpVidMem - _DS(LFBBASE);
#endif
    }
    // src is in system memory
    srcBaseAddr = pDDSrcSurf->lpGbl->fpVidMem;
#if ENABLE_TILED_HEAP
    if(IS_TILED(dstBaseAddr))
    {
      dstPitch = _DS(ddTileStride);
    }
    else
#endif
#if ENABLE_3D
    if ((FOURCC_DXT1 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
        (FOURCC_FXT1 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC))
    {
      dstPitch = (((pDstRect->right - pDstRect->left) * 4) + 7) / 8;
    }
    else if ((FOURCC_DXT2 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT3 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT4 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT5 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC))
    {
      dstPitch = (((pDstRect->right - pDstRect->left) * 8) + 7) / 8;
    }
    else
#endif
    {
      dstPitch = pDDDstSurf->lpGbl->lPitch;
    }

#if ENABLE_3D
    if ((FOURCC_DXT1 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
        (FOURCC_FXT1 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC))
    {
      bitsPerPel = 4;

      width = (((pDstRect->right - pDstRect->left) * bitsPerPel) + 7) / 8;
      height = pDstRect->bottom - pDstRect->top;
      if (width < 4)
      {

         if (width && ((height / width) <= 16))
         {  // Aspect ratio is <= 8:1
            int size;
            DWORD *src;
            DWORD *dst;
            // Lets go ahead and stretch DXT1 textures out to 8 wide before we copy them.
#ifdef WINNT
// W2K's DX7 runtime appears to munge the width and height of system memory
// DXTn surfaces. Since src and dst surfaces are the same size, we'll use 
// pDDDstSurf's fields instead. This fixes ground corruption in 3D Mark 2000's
// RustValley test.
            size = (pDDDstSurf->lpGbl->wHeight >> 2) << 2;
#else
            size = (pDDSrcSurf->lpGbl->wHeight >> 2) << 2;
#endif
            src = (DWORD*)(srcBaseAddr) + ((size >> 1) - 2);
            dst = (buffer + (size -4));
            while (size >= 4)
            {
               dst[0] = src[0];
               dst[1] = src[1];
               dst -= 4;
               src -= 2;
               size -= 4;
            }
            srcBaseAddr = (DWORD)buffer;
            dstPitch = 4;
            width = 4;
         }
         else
         {  // We can do it this way because it will be strected in width latter.

            // Because the data transfer loop bellow expects to always move dwords,
            // make sure the width and pitch are always at least 4 so there is a dword
            // to move, otherwise we move nothing height number of times and the CMDFIFO
            // get off.
            if (width == 1)
            {
               dstPitch <<=2;
               width <<=2;
               height >>=2;
            }
            else
            {
               dstPitch <<=1;
               width <<=1;
               height >>=1;
            }
            if (height == 0)
               height = 1;
            // Note: that forcing height to 1 may cause more data to be copied than is present,
            // ie. there is not a full dword to move.  This should be OK because of allocation
            // routines forcing things to dword or larger boundaries.
         }
      }
      srcPitch = width;
    }
    else if ((FOURCC_DXT2 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT3 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT4 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC) ||
             (FOURCC_DXT5 == pDDDstSurf->lpGbl->ddpfSurface.dwFourCC))
    {
      bitsPerPel = 8;

      width = (((pDstRect->right - pDstRect->left) * bitsPerPel) + 7) / 8;
      height = pDstRect->bottom - pDstRect->top;

      srcPitch = width;
    }
    else
#endif
    {
      bitsPerPel = pDDDstSurf->lpGbl->ddpfSurface.dwRGBBitCount;

      width = (((pDstRect->right - pDstRect->left) * bitsPerPel) + 7) / 8;
      height = pDstRect->bottom - pDstRect->top;

      srcPitch = pDDSrcSurf->lpGbl->lPitch;
    }

    BLTCLIP(0, 0, clip1min);
    BLTCLIP(width, height, clip1max);

    BLTFMT(dstPitch, SSTG_PIXFMT_8BPP, dstFormat);
    BLTFMT(srcPitch, SSTG_PIXFMT_8BPP, srcFormat);

    BLTSIZE(width, height, dstSize);

    CMDFIFO_CHECKROOM(hwPtr, bumpNum);

    SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstBaseAddrBit |
                                   dstFormatBit   |
                                   srcBaseAddrBit |
                                   clip1minBit    |
                                   clip1maxBit    |
                                   srcFormatBit   |
                                   srcXYBit       |
                                   dstSizeBit     |
                                   dstXYBit       |
                                   commandBit));
    SETPD(hwPtr, ghw2D->dstBaseAddr, dstBaseAddr);
    SETPD(hwPtr, ghw2D->dstFormat,   dstFormat);
    SETPD(hwPtr, ghw2D->srcBaseAddr, srcBaseAddr);
    SETPD(hwPtr, ghw2D->clip1min,    clip1min);
    SETPD(hwPtr, ghw2D->clip1max,    clip1max);
    SETPD(hwPtr, ghw2D->srcFormat,   srcFormat);
    SETPD(hwPtr, ghw2D->srcXY,       0);
    SETPD(hwPtr, ghw2D->dstSize,     dstSize);
    SETPD(hwPtr, ghw2D->dstXY,       0);
    SETPD(hwPtr, ghw2D->command,     SSTG_ROP_SRCCOPY |
                                     SSTG_HOST_BLT    |
                                     SSTG_CLIPSELECT);

    BUMP(bumpNum);

    // compute the number of dwords to write per scanline
    // we're faking the hw into assuming all data is 8bpp
    numDwords = (width * 1 + 3) / 4;

    for (i = 0; i < height; i++)
    {
      scaneline = (DWORD *)srcBaseAddr;
      CMDFIFO_CHECKROOM(hwPtr, numDwords + 1);
      SETPH(hwPtr, CMDFIFO_BUILD_2DPK1(numDwords, 0, launch[0], 0xF));
      for (j = 0; j < numDwords; j++)
      {
        SETPD(hwPtr, ghw2D->launch[0], *scaneline);
        scaneline++;
      }
      srcBaseAddr += srcPitch;
      BUMP(numDwords + 1);
    }


#if defined(WINNT) && (_WIN32_WINNT < 0x0500)
    // restore nt invariant regs
    CMDFIFO_CHECKROOM(hwPtr, 5);
    SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstBaseAddrBit |
                                   dstFormatBit   |
                                   srcBaseAddrBit |
                                   srcFormatBit));
    SETPD(hwPtr, ghw2D->dstBaseAddr, ppdev->ulScreenOffset);
    SETPD(hwPtr, ghw2D->dstFormat,   ppdev->ulScreenFormat);
    SETPD(hwPtr, ghw2D->srcBaseAddr, ppdev->ulScreenOffset);
    SETPD(hwPtr, ghw2D->srcFormat,   ppdev->ulScreenFormat);
    BUMP(5);
#endif

    CMDFIFO_EPILOG(hwPtr);

#if ENABLE_3D
    if (pDDDstSurf->ddsCaps.dwCaps & DDSCAPS_TEXTURE)
    {
      TXTRDESC *txtr;
      txtr = TXTRDESC_PTR(pDDDstSurf->dwReserved1);

      txtr->flags |= BltToTxtrInFifo;

      if (TEXTURE_IS_DXT_SURFACE(pDDDstSurf->lpGbl->ddpfSurface)
          && txtr->flags & AspectRatioGT8)
      {
         // Because we are about to go access the frame buffer to modify the
         // data that was just blit there, we need to make sure the data is
         // finished getting there, so we wait until the RE is finished with
         // the BLT.
         FXBUSYWAIT(ppdev);
         MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);
         stretchDXTn(pDDDstSurf->lpGbl,txtr,&_D3G,ppdev);
         if (pDDDstSurf->lpGbl->ddpfSurface.dwFlags & DDPF_FOURCC
             && pDDDstSurf->lpGbl->ddpfSurface.dwFourCC == FOURCC_DXT1)
         {
            if (txtr->tlog > txtr->slog)
            {
               int widthshift = txtr->initialTlog - txtr->initialSlog - 3;
               if ((pDDDstSurf->lpGbl->wWidth << widthshift) < 8)
                  PAD_DXT1(ppdev, pDDDstSurf->lpGbl);
            }
         MODIFY_SLI_READ(ppdev, DISABLE_SLI_READ);
         }
      }
    }
#endif
  }

  return DD_OK;
} // Blt32_CopyFourCC


#if ENABLE_3D && (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
/*----------------------------------------------------------------------
Function name:  Blt32_TexBltCopyFourCC()

Description:    retro3dfx: 7-argument adapter so the D3DDP2OP_TEXBLT
                handler can invoke Blt32_CopyFourCC through the same
                PTEXBLTFUNC signature as textureLoad().

                The vintage code cast the 5-argument __stdcall
                Blt32_CopyFourCC directly to the 7-argument PTEXBLTFUNC
                and called it with (ppdev, TXTRHNDL*, RECTL*, int LOD,
                TXTRHNDL*, RECTL*, int LOD).  Argument 4 (nSrcLOD, an
                integer, normally 0) therefore arrived in
                Blt32_CopyFourCC's pDDDstSurf parameter and was
                dereferenced as a surface pointer -> bugcheck 1000008E
                on the first managed FourCC (DXTn) texture blt (seen
                with UT2004's D3D renderer).  The callee also popped
                0x14 bytes while the caller pushed 0x1c.

                This wrapper rebuilds the DDRAWI surface views that
                Blt32_CopyFourCC expects from the TXTRHNDL's per-LOD
                mipmap data (populated in D7D3D.C when the runtime
                associates the surface with its handle) and calls it
                with the correct arity.  The rectangles passed down are
                the full LOD level, per the DDK contract for FourCC
                copies (no stretching, no sub-rectangles); prSrc/prDest
                from the DP2 stream are ignored because the TEXBLT
                mip-loop halves them from the runtime-munged system
                memory dimensions, which do not survive reinterpretation
                through the DXT bits-per-pixel math in Blt32_CopyFourCC.

Return:         DD_OK or DDERR_*
----------------------------------------------------------------------*/

DWORD __stdcall
Blt32_TexBltCopyFourCC(NT9XDEVICEDATA  *ppdev,
                       TXTRHNDL        *pSrcSurf,
                       RECTL           *prSrc,
                       int              nSrcLOD,
                       TXTRHNDL        *pDstSurf,
                       RECTL           *prDest,
                       int              nDstLOD)
{
  DD_SURFACE_GLOBAL         gblSrc, gblDst;
  DD_SURFACE_LOCAL          lclSrc, lclDst;
  DD_SURFACE_MORE           moreSrc, moreDst;
  RECTL                     rSrcFull, rDstFull;
  MIPMAPDATA                mmSrc, mmDst;
  DWORD                     retval;

  prSrc;   // unreferenced, see above
  prDest;  // unreferenced, see above

  if ((NULL == pSrcSurf) || (NULL == pDstSurf))
    return DDERR_INVALIDPARAMS;

  // Pick the per-LOD source/dest data.  mmData[0] is valid for any
  // texture surface (the population loop in D7D3D.C always runs at
  // least once); fall back to the top-level fields for LOD 0 of a
  // TXTRHNDL without mipmap data.

  if ((nSrcLOD >= 0) && (nSrcLOD < pSrcSurf->nLevels))
    mmSrc = pSrcSurf->mmData[nSrcLOD];
  else if (0 == nSrcLOD)
  {
    mmSrc.wWidth   = pSrcSurf->wWidth;
    mmSrc.wHeight  = pSrcSurf->wHeight;
    mmSrc.lPitch   = pSrcSurf->lPitch;
    mmSrc.fpVidMem = pSrcSurf->fpVidMem;
  }
  else
    return DDERR_INVALIDPARAMS;

  if ((nDstLOD >= 0) && (nDstLOD < pDstSurf->nLevels))
    mmDst = pDstSurf->mmData[nDstLOD];
  else if (0 == nDstLOD)
  {
    mmDst.wWidth   = pDstSurf->wWidth;
    mmDst.wHeight  = pDstSurf->wHeight;
    mmDst.lPitch   = pDstSurf->lPitch;
    mmDst.fpVidMem = pDstSurf->fpVidMem;
  }
  else
    return DDERR_INVALIDPARAMS;

  memset(&gblSrc,  0, sizeof(gblSrc));
  memset(&gblDst,  0, sizeof(gblDst));
  memset(&lclSrc,  0, sizeof(lclSrc));
  memset(&lclDst,  0, sizeof(lclDst));
  memset(&moreSrc, 0, sizeof(moreSrc));
  memset(&moreDst, 0, sizeof(moreDst));

  // dwReserved1 of the GBL views stays 0 so Blt32_CopyFourCC addresses
  // through fpVidMem (the per-LOD address) rather than the
  // surface-wide FXSURFACEDATA hwPtr.

  gblSrc.fpVidMem                  = (FLATPTR)mmSrc.fpVidMem;
  gblSrc.lPitch                    = mmSrc.lPitch;
  gblSrc.wWidth                    = mmSrc.wWidth;
  gblSrc.wHeight                   = mmSrc.wHeight;
  gblSrc.ddpfSurface.dwSize        = sizeof(gblSrc.ddpfSurface);
  gblSrc.ddpfSurface.dwFlags       = pSrcSurf->dwFlags;
  gblSrc.ddpfSurface.dwFourCC      = pSrcSurf->dwFourCC;
  gblSrc.ddpfSurface.dwRGBBitCount = pSrcSurf->dwBitCnt;

  lclSrc.lpGbl          = &gblSrc;
  lclSrc.ddsCaps.dwCaps = pSrcSurf->dwCaps;
  lclSrc.dwReserved1    = pSrcSurf->txtrID;
  lclSrc.lpSurfMore     = &moreSrc;

  gblDst.fpVidMem                  = (FLATPTR)mmDst.fpVidMem;
  gblDst.lPitch                    = mmDst.lPitch;
  gblDst.wWidth                    = mmDst.wWidth;
  gblDst.wHeight                   = mmDst.wHeight;
  gblDst.ddpfSurface.dwSize        = sizeof(gblDst.ddpfSurface);
  gblDst.ddpfSurface.dwFlags       = pDstSurf->dwFlags;
  gblDst.ddpfSurface.dwFourCC      = pDstSurf->dwFourCC;
  gblDst.ddpfSurface.dwRGBBitCount = pDstSurf->dwBitCnt;

  lclDst.lpGbl          = &gblDst;
  lclDst.ddsCaps.dwCaps = pDstSurf->dwCaps;
  lclDst.dwReserved1    = pDstSurf->txtrID;   // TXTRDESC handle for the
                                              // DXT1 stretch/pad fixups
  lclDst.lpSurfMore     = &moreDst;

  rSrcFull.left = rSrcFull.top = 0;
  rSrcFull.right  = (LONG)mmSrc.wWidth;
  rSrcFull.bottom = (LONG)mmSrc.wHeight;

  rDstFull.left = rDstFull.top = 0;
  rDstFull.right  = (LONG)mmDst.wWidth;
  rDstFull.bottom = (LONG)mmDst.wHeight;

  retval = Blt32_CopyFourCC(ppdev, &lclSrc, &rSrcFull, &lclDst, &rDstFull);

#if ENABLE_LOG_FILE
  if (DD_OK != retval)
    retroLogForce(ppdev, "retro3dfx TEXBLT-4CC-ERR: hr=%08lXh fourcc=%08lXh lod=%d/%d dst=%ldx%ld\r\n",
                  retval, pDstSurf->dwFourCC, nSrcLOD, nDstLOD,
                  (LONG)mmDst.wWidth, (LONG)mmDst.wHeight);
#endif

  return retval;
} // Blt32_TexBltCopyFourCC
#endif // ENABLE_3D && DX7

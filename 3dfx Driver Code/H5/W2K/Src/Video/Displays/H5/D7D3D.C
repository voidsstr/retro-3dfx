/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** $Revision: 22$
** $Date: 10/31/00 2:02:04 AM$
**
*/

/**************************************************************************
*
* DIRECTDRAW FUNCTIONS:
*
* ddiGetDriverState    --- DX7 GetDriverState entry point
* ddiCreateSurfaceEx   --- DX7 CreateSurfaceEx entry point
* ddiDestroyDDLocal    --- DX7 DestroyDDLocal entry point
*
* GetHndlListPtr
*
* INTERNAL FUNCTIONS:
*
* AllocHndlList
* ReleaseHndlList
* GetTxtrHndl
* AllocTxtrHndl
* ReleaseTxtrHndl
*
***************************************************************************/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

#include "precomp.h"

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)

#ifndef WINNT
// Everything NT builds need are in precomp.h!
#include "d3txtr.h"
#include "d3contxt.h"
#include "ddglobal.h"
#endif

#define DDSCAPS2_HINTDYNAMIC    0x00000004L
#define DDSCAPS2_HINTSTATIC     0x00000008L

#define DDSCAPS_EXECUTEBUFFER   DDSCAPS_RESERVED2
#define DDSCAPS2_VERTEXBUFFER   DDSCAPS2_RESERVED1
#define DDSCAPS2_COMMANDBUFFER  DDSCAPS2_RESERVED2
#define DDSCAPS2_INDEXBUFFER    DDSCAPS2_RESERVED3

#ifdef WINNT
// due to calling sequence on W2K, we may need to have the head of
// the linked list as a global variable (similar to the RC linked list)
HNDLLIST *g_pHndlList;
#else
#define g_pHndlList   _D3(pHndlList)
#endif

/**************************************************************************
* D E F I N E S
***************************************************************************/

#define ENTRY_EXIT_DBG_LEVEL    2
#define NORMAL_DBG_LEVEL        2

/**************************************************************************
* F U N C T I O N   P R O T O T Y P E S
***************************************************************************/

// because of the weird way these functions are placed in d3txtr.h
// we need to prototype them here
extern signed int __stdcall ALLOCTEXTUREHANDLE(NT9XDEVICEDATA * ppdev, TXTRHNDL *txtrList);
extern void __stdcall FREETEXTUREHANDLE(NT9XDEVICEDATA * ppdev, int);

static HNDLLIST *AllocHndlList(NT9XDEVICEDATA*, LPVOID);
static VOID ReleaseHndlList(NT9XDEVICEDATA*, HNDLLIST*);

static DWORD GetTxtrHndl(NT9XDEVICEDATA*,LPVOID,HNDLLIST*,DWORD);
static DWORD AllocTxtrHndl(NT9XDEVICEDATA*, LPVOID, HNDLLIST*, DWORD);

/**************************************************************************
* P U B L I C   F U N C T I O N S
***************************************************************************/

/*----------------------------------------------------------------------
Function name: ddiGetDriverState

Description:   DX7 Callback GetDriverState()

   This callback is used by both the DirectDraw and Direct3D runtimes to obtain
   information from the driver about its current state.

   Parameters

       pgdsd
             pointer to GetDriverState data structure

             dwFlags
                     Flags to indicate the data required
             dwhContext
                     The ID of the context for which information
                     is being requested
             lpdwStates
                     Pointer to the state data to be filled in by the driver
             dwLength
                     Length of the state data buffer to be filled
                     in by the driver
             ddRVal
                     Return value

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
               DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
ddiGetDriverState( LPDDHAL_GETDRIVERSTATEDATA pgdsd )
{
  D3DPRINT(ENTRY_EXIT_DBG_LEVEL,">> ddiGetDriverState");

  pgdsd->ddRVal = DD_OK;

  D3DPRINT(ENTRY_EXIT_DBG_LEVEL,"<< ddiGetDriverState");

  return DDHAL_DRIVER_HANDLED;
} // ddiGetDriverState

/*----------------------------------------------------------------------
Function name: ddiCreateSurfaceEx

Description:   DX7 Callback CreateSurfaceEx()

   ddiCreateSurfaceEx creates a Direct3D surface from a DirectDraw surface and
   associates a requested handle value to it.

   All Direct3D drivers must support ddiCreateSurfaceEx.

   ddiCreateSurfaceEx creates an association between a DirectDraw surface and
   a small integer surface handle. By creating these associations between a
   handle and a DirectDraw surface, ddiCreateSurfaceEx allows a surface handle
   to be imbedded in the Direct3D command stream. For example when the
   D3DDP2OP_TEXBLT command token is sent to ddiDrawPrimitives2 to load a texture
   map, it uses a source handle and destination handle which were associated
    with a DirectDraw surface through ddiCreateSurfaceEx.

   For every DirectDraw surface created under the local DirectDraw object, the
   runtime generates a valid handle that uniquely identifies the surface and
   places it in pcsxd->lpDDSLcl->lpSurfMore->dwSurfaceHandle. This handle value
   is also used with the D3DRENDERSTATE_TEXTUREHANDLE render state to enable
   texturing, and with the D3DDP2OP_SETRENDERTARGET and D3DDP2OP_CLEAR commands
   to set and/or clear new rendering and depth buffers. The driver should fail
   the call and return DDHAL_DRIVER_HANDLE if it cannot create the Direct3D
   surface. If the DDHAL_CREATESURFACEEX_SWAPHANDLES flag is set, the handles
   should be swapped over two sequential calls to ddiCreateSurfaceEx.
   As appropriate, the driver should also store any surface-related information
   that it will subsequently need when using the surface. The driver must create
   a new surface table for each new lpDDLcl and implicitly grow the table when
   necessary to accommodate more surfaces. Typically this is done with an
   exponential growth algorithm so that you don't have to grow the table too
   often. Direct3D calls ddiCreateSurfaceEx after the surface is created by
   DirectDraw by request of the Direct3D runtime or the application.

   Parameters

        pcsxd
             pointer to CreateSurfaceEx structure that contains the information
             required for the driver to create the surface (described below).

             dwFlags
                     May have the value(s):
                     DDHAL_CREATESURFACEEX_SWAPHANDLES
                                If this flag is set, ddiCreateSurfaceEx will be
                                called twice, with different values in lpDDSLcl
                                in order to swap the associated texture handles
             lpDDLcl
                     Handle to the DirectDraw object created by the application.
                     This is the scope within which the lpDDSLcl handles exist.
                     A DD_DIRECTDRAW_LOCAL structure describes the driver.
             lpDDSLcl
                     Handle to the DirectDraw surface we are being asked to
                     create for Direct3D. These handles are unique within each
                     different DD_DIRECTDRAW_LOCAL. A DD_SURFACE_LOCAL structure
                     represents the created surface object.
             ddRVal
                     Specifies the location in which the driver writes the return
                     value of the ddiCreateSurfaceEx callback. A return code of
                     DD_OK indicates success.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
               DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
ddiCreateSurfaceEx( LPDDHAL_CREATESURFACEEXDATA pcsxd )
{
  NT9XDEVICEDATA              *ppdev;
  LPVOID                      pDDLcl  = (LPVOID)pcsxd->lpDDLcl;
  LPDDRAWI_DDRAWSURFACE_LCL   pDDSLcl = pcsxd->lpDDSLcl;
  LPDDRAWI_DDRAWSURFACE_LCL   pCurr = pDDSLcl;
  LPATTACHLIST                curr;
  HNDLLIST                    *pHndlList;
  TXTRHNDL                    *pTxtrHndl;
  DWORD                       dwTxtrHndl;


  D3DPRINT(ENTRY_EXIT_DBG_LEVEL,">> ddiCreateSurfaceEx");

  pcsxd->ddRVal = DD_OK;

  if (NULL == pDDSLcl || NULL == pDDLcl)
  {
    D3DPRINT(0,"ddiCreateSurfaceEx received NULL pDDLcl or pDDSLcl pointer");
    return DDHAL_DRIVER_HANDLED;
  }

  // We check that what we are handling is a texture, zbuffer or a rendering
  // target buffer. We don't check if it is however stored in local video
  // memory since it might also be a system memory texture that we will later
  // blt with __TextureBlt.
  // also if your driver supports DDSCAPS_EXECUTEBUFFER create itself, it must
  // process DDSCAPS_EXECUTEBUFFER here as well.
#if (DIRECT3D_VERSION < 0x0800) && (DX < 8)
#if defined(TnL_HAL) && defined(VERT_BUFF)
  if (!(pDDSLcl->ddsCaps.dwCaps & (DDSCAPS_TEXTURE  |
                                   DDSCAPS_3DDEVICE |
                                   DDSCAPS_ZBUFFER  |
                                   DDSCAPS_EXECUTEBUFFER)))
#else

  if (!(pDDSLcl->ddsCaps.dwCaps & (DDSCAPS_TEXTURE  |
                                   DDSCAPS_3DDEVICE |
                                   DDSCAPS_ZBUFFER)))
#endif // TnL_HAL & Vertex Buffer support
  {
    D3DPRINT(NORMAL_DBG_LEVEL,"ddiCreateSurfaceEx w/o "
                              "DDSCAPS_TEXTURE/3DDEVICE/ZBUFFER Ignored, "
                              "dwCaps=%8lXh dwSurfaceHandle=%ld",
             pDDSLcl->ddsCaps.dwCaps,
             pDDSLcl->lpSurfMore->dwSurfaceHandle);

    // fix for PRS 14511, clear stale TXTRHNDL's
    if (0 != pDDSLcl->lpSurfMore->dwSurfaceHandle)
    {
      ppdev = (NT9XDEVICEDATA *)pcsxd->lpDDLcl->lpGbl->dhpdev;
      pHndlList = GetHndlListPtr(ppdev, pDDLcl);
      if (NULL != pHndlList)
      {
        dwTxtrHndl = GetTxtrHndl(ppdev, pDDLcl, pHndlList, pDDSLcl->lpSurfMore->dwSurfaceHandle);
        if (0 != dwTxtrHndl)
        {
          D3DPRINT(NORMAL_DBG_LEVEL, "  releasing TXTRHNDL %ld for ignored surface", dwTxtrHndl);
          ReleaseTxtrHndl(ppdev, pHndlList, dwTxtrHndl);
        }
      }
    }

    return DDHAL_DRIVER_HANDLED;
  }
#endif // DX8

#ifdef DEBUG
  if (pDDSLcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE)
  {
      D3DPRINT(17, "ddiCreateSurfaceEx: TEXTURE, dwCaps=%8lXh dwSurfaceHandle=%ld",
             pDDSLcl->ddsCaps.dwCaps,
             pDDSLcl->lpSurfMore->dwSurfaceHandle);
  }
  if (pDDSLcl->ddsCaps.dwCaps & DDSCAPS_3DDEVICE)
  {
      D3DPRINT(17, "ddiCreateSurfaceEx: 3DDEVICE, dwCaps=%8lXh dwSurfaceHandle=%ld",
             pDDSLcl->ddsCaps.dwCaps,
             pDDSLcl->lpSurfMore->dwSurfaceHandle);
  }
  if (pDDSLcl->ddsCaps.dwCaps & DDSCAPS_ZBUFFER)
  {
      D3DPRINT(17, "ddiCreateSurfaceEx: ZBUFFER, dwCaps=%8lXh dwSurfaceHandle=%ld",
             pDDSLcl->ddsCaps.dwCaps,
             pDDSLcl->lpSurfMore->dwSurfaceHandle);
  }
#endif

#ifdef WINNT
  ppdev = (NT9XDEVICEDATA *)pcsxd->lpDDLcl->lpGbl->dhpdev;
#else
  ppdev = (NT9XDEVICEDATA *)pcsxd->lpDDLcl->lpGbl->dwReserved3;
#endif

  // Now allocate the texture data space
  do
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "pDDLcl = %8lXh, pDDSLcl = %8lXh, dwHandle = %ld, dwCaps = %08lXh",
             pDDLcl, pDDSLcl, pDDSLcl->lpSurfMore->dwSurfaceHandle, pDDSLcl->ddsCaps.dwCaps);

#if defined(WINNT) && DBG
    // print out surface desc info for system memory surfaces
    if (DDSCAPS_SYSTEMMEMORY & pDDSLcl->ddsCaps.dwCaps)
    {
      extern VOID DUMP_SURFACEINFO(NT9XDEVICEDATA*,LPDDRAWI_DDRAWSURFACE_LCL);
      DUMP_SURFACEINFO(ppdev,pDDSLcl);
      if (DDSCAPS_MIPMAP & pDDSLcl->ddsCaps.dwCaps)
      {
        LPDDRAWI_DDRAWSURFACE_LCL pSurf = pDDSLcl;
        int i;

        D3DPRINT(NORMAL_DBG_LEVEL, "    mipmap count = %8lXh", pDDSLcl->lpSurfMore->dwMipMapCount);

        i = 0;
        while (NULL != pSurf)
        {
          D3DPRINT(NORMAL_DBG_LEVEL,"      surf=%d pSurf=%8lXh handle=%4ld fpVidMem=%8lXh lPitch=%8lXh, width=%8lXh, height=%8lXh",
                   i++, pSurf, pSurf->lpSurfMore->dwSurfaceHandle, pSurf->lpGbl->fpVidMem, pSurf->lpGbl->lPitch, pSurf->lpGbl->wWidth, pSurf->lpGbl->wHeight);

          if (NULL != pSurf->lpAttachList)
            pSurf = pSurf->lpAttachList->lpAttached;
          else
            pSurf = NULL;
        }
      }
    }
#endif

    if (0 == pDDSLcl->lpSurfMore->dwSurfaceHandle)
    {
      D3DPRINT(0,"ddiCreateSurfaceEx got 0 dwSurfaceHandle, dwCaps=%08lXh",
               pDDSLcl->ddsCaps.dwCaps);
#if ENABLE_LOG_FILE
      retroLogForce(ppdev, "retro3dfx CSEX-ZEROHANDLE: caps=%08lXh\r\n",
                    pDDSLcl->ddsCaps.dwCaps);
#endif
      break;
    }

    // find or allocate the HNDLLIST associated with this pDDLcl
    pHndlList = GetHndlListPtr(ppdev, pDDLcl);
    if (NULL == pHndlList)
    {
      // a HNDLLIST isn't already associated with the pDDLcl so create one now
      pHndlList = AllocHndlList(ppdev, pDDLcl);
      if (NULL == pHndlList)
      {
        D3DPRINT(0, "  unable to allocate a HndlList for pDDLcl=%8lXh",pDDLcl);
        pcsxd->ddRVal = DDERR_OUTOFMEMORY;
        break;
      }
    }
    D3DPRINT(NORMAL_DBG_LEVEL, "  pHndlList = %8lXh", pHndlList);

    // for DX7 do what ddiHandleCreate does here, except that we don't know
    // the context yet

    // find or allocate the TXTRHNDL for this pDDLcl-Handle pair
    dwTxtrHndl = GetTxtrHndl(ppdev, pDDLcl, pHndlList, pDDSLcl->lpSurfMore->dwSurfaceHandle);
    if (0 == dwTxtrHndl)
    {
      // a TXTRHNDL doesn't exist for this pDDLcl-Handle pair
      // so allocate one now
      dwTxtrHndl = AllocTxtrHndl(ppdev, pDDLcl, pHndlList, pDDSLcl->lpSurfMore->dwSurfaceHandle);
      if (0 == dwTxtrHndl)
      {
        D3DPRINT(0, "  unable to allocate a TXTRHNDL for handle %ld, pDDLcl = %8lXh",
                 pDDSLcl->lpSurfMore->dwSurfaceHandle, pDDLcl);
        pcsxd->ddRVal = DDERR_OUTOFMEMORY;
        break;
      }
    }

    // init whatever TXTRHNDL data we can here
    pTxtrHndl = pHndlList->ppTxtrHndlList[dwTxtrHndl];

    pTxtrHndl->surfData = (FXSURFACEDATA *) pDDSLcl->lpGbl->dwReserved1;
    pTxtrHndl->txtrID   = pDDSLcl->dwReserved1;
    pTxtrHndl->dwFlags  = pDDSLcl->lpGbl->ddpfSurface.dwFlags;
    pTxtrHndl->dwFourCC = pDDSLcl->lpGbl->ddpfSurface.dwFourCC;
    pTxtrHndl->dwCaps   = pDDSLcl->ddsCaps.dwCaps;
    pTxtrHndl->wWidth   = pDDSLcl->lpGbl->wWidth;
    pTxtrHndl->wHeight  = pDDSLcl->lpGbl->wHeight;
    pTxtrHndl->lPitch   = pDDSLcl->lpGbl->lPitch;
    pTxtrHndl->dwZMask  = pDDSLcl->lpGbl->ddpfSurface.dwZBitMask;
    pTxtrHndl->dwZDepth = pDDSLcl->lpGbl->ddpfSurface.dwZBufferBitDepth;
    pTxtrHndl->dwStencil= pDDSLcl->lpGbl->ddpfSurface.dwStencilBitMask;
    pTxtrHndl->dwBitCnt = pDDSLcl->lpGbl->ddpfSurface.dwRGBBitCount;
	pTxtrHndl->dwRBitMask = pDDSLcl->lpGbl->ddpfSurface.dwRBitMask;
    pTxtrHndl->fpVidMem = pDDSLcl->lpGbl->fpVidMem;

    pTxtrHndl->dwColorSpaceLowValue  = pDDSLcl->ddckCKSrcBlt.dwColorSpaceLowValue;
    pTxtrHndl->dwColorSpaceHighValue = pDDSLcl->ddckCKSrcBlt.dwColorSpaceHighValue;

    pTxtrHndl->nLevels = 0;

    // we'll attempt to fill in the contextId here,
    // but if no context exists for this pDDLcl then we'll
    // fill in the contextId in ddiContextCreate
    if (DDSCAPS_TEXTURE & pDDSLcl->ddsCaps.dwCaps)
    {
      // update palette handle on texture swaps
      if ((pDDSLcl->dwReserved1) && PALETTIZED(pDDSLcl->dwReserved1))
      {
        TXTRDESC  *pTxtrDesc;

        pTxtrDesc = TXTRDESC_PTR(pDDSLcl->dwReserved1);
        if (pTxtrDesc->dwPaletteHandle)
        {
          D3DPRINT(NORMAL_DBG_LEVEL, "  updating palette handle for TxtrHndl[%ld] from %ld to %ld",
                   dwTxtrHndl, pTxtrHndl->dwPaletteHandle, pTxtrDesc->dwPaletteHandle);
          pTxtrHndl->dwPaletteHandle = pTxtrDesc->dwPaletteHandle;
        }
      }

#if RC_LINKED_LIST
      // walk context linked list
      {
        RC  *pRc = (RC *)&g_pContexts;

        while (pRc->pNext)
        {
          pRc = pRc->pNext;

          // if the context pDDLcl is this calls pDDLcl
          // update the TXTRHNDL contextId
          if (pRc->pDDLcl == pDDLcl)
          {
            pTxtrHndl->contextId = (DWORD)pRc;
            D3DPRINT(NORMAL_DBG_LEVEL, "  setting TxtrHndl[%ld] contextId to %8lXh",
                     dwTxtrHndl, pRc);

            // if this is the current texture then indicate this change to the rendering code
            if (dwTxtrHndl == pRc->texture)
            {
              UPDATE_HW_STATE(SC_SOMETHING);
            }
            break;
          }
        }
      }
#else
      {
        DWORD handle;

        // loop over context array
        for (handle = 0; handle < NUMCONTEXTS; handle++)
        {
            RC  *pRc = CONTEXT_INDEX(handle);


            // if the context is in use and the context pDDLcl is this calls pDDLcl
            // update the TXTRHNDL contextId
            if (CONTEXT_MAP(handle) && pRc->pDDLcl == pDDLcl)
            {
                pTxtrHndl->contextId = (DWORD)pRc;
                D3DPRINT(NORMAL_DBG_LEVEL, "  setting TxtrHndl[%ld] contextId to %8lXh",
                           dwTxtrHndl, pRc);

                // if this is the current texture then indicate this change to the rendering code
                if (dwTxtrHndl == pRc->texture)
                {
                    UPDATE_HW_STATE(SC_SOMETHING);
                }
                break;
            }
        }
      }
#endif

      // this is a texture surface get the mipmap information
      do
      {
        // Save information about this mipmap

        pTxtrHndl->mmData[pTxtrHndl->nLevels].wWidth =     pCurr->lpGbl->wWidth;
        pTxtrHndl->mmData[pTxtrHndl->nLevels].wHeight =    pCurr->lpGbl->wHeight;
        pTxtrHndl->mmData[pTxtrHndl->nLevels].lPitch =     pCurr->lpGbl->lPitch;
        pTxtrHndl->mmData[pTxtrHndl->nLevels].fpVidMem =   (unsigned long) pCurr->lpGbl->fpVidMem;

        // Step to the next mipmap level, if any
        if (pCurr->lpAttachList)
            pCurr = pCurr->lpAttachList->lpAttached;
        else
            pCurr = NULL;

        pTxtrHndl->nLevels++;

      } while (pCurr && (pTxtrHndl->nLevels < (int) MAX_MIPMAP_LEVELS));
    }

    if ((DDSCAPS_SYSTEMMEMORY & pDDSLcl->ddsCaps.dwCaps) &&
        (0 == pDDSLcl->lpGbl->fpVidMem))
    {
      // this is a system memory surface that is being destroyed
      // so clear the TXTRHNDL
      D3DPRINT(NORMAL_DBG_LEVEL, "  system surface being destroyed, clearing TXTRHNDL %ld", dwTxtrHndl);
      memset(pHndlList->ppTxtrHndlList[dwTxtrHndl], 0, sizeof(TXTRHNDL));
    }

    // for some surfaces other than MIPMAP or CUBEMAP, such as flipping chains,
    // we make a slot for every surface, as they are not as interleaved
    if ((DDSCAPS_MIPMAP & pDDSLcl->ddsCaps.dwCaps) ||
        (DDSCAPS2_CUBEMAP & pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2))
      break;
    curr = pDDSLcl->lpAttachList;
WORKAROUND_W2K_ATTACHED_SURFACE_LIST_ANOMALY:
    if (NULL == curr)
      break;
    pDDSLcl = curr->lpAttached;

    // Workaround for Pod Racer attached surface list problem (This appears to be a bug in the OS!)
    // PRS 12121
    //
    // Pod Racer creates a flipping chain consisting of a primary and one back buffer.
    // At that point, the primary's lpAttachList->lpAttached is the back buffer's SURFACE_LOCAL
    // and the primary's lpAttachList->lpLink is NULL.
    // Then Pod Racer creates a z buffer.  The z buffer's lpAttachList is NULL at that point.
    // Then DdAddAttachedSurface is called with the primary's SURFACE_LOCAL as pasd->lpDDSurface
    // and the z buffer's SURFACE_LOCAL as pasd->lpSurfAttached.  At that point, none of the attached
    // lists have been changed from when DdCreateSurface was called.
    // Next Pod Racer creates 150 or so system memory texture surfaces.
    // Then the z buffer surface is destroyed.  Somewhere between the AddAttachedSurface call and the
    // DdDestroySurface call.  The z buffer and primary attachment list have been modified.  The z buffer
    // has the primary's SURFACE_LOCAL as lpAttachedListFrom->lpAttached.  And the primary has the
    // z buffer's SURFACE_LOCAL as lpAttachList->lpAttached and a non NULL lpAttachList->lpLink.  The
    // primary's lpAttachList->lpLink->lpAttached contains the SURFACE_LOCAL of the back buffer.
    // This attachment list doesn't seem to change here after.
    // Next Pod Racer destroys the backbuffer, and then destroys the primary.
    // Then the flipping chain is recreated, but the attachment lists are the same as when the zbuffer
    // was destroyed (even though, there is no z buffer surface at this point!)
    // When ddiCreateSurfaceEx is called after the flipping chain is recreated, the attachment list
    // incorrectly has the z buffer that will be created shortly as curr->lpAttached.  And the back
    // buffer that was just created is in curr->lpLink->lpAttached.  So in order to correctly initialize
    // the TXTRHNDL for the recreated back buffer, we need to do the following:

    // see if this surface was created yet
    if ((0 == pDDSLcl->lpGbl->dwReserved1) && (DDSCAPS_VIDEOMEMORY & pDDSLcl->ddsCaps.dwCaps))
    {
      D3DPRINT(NORMAL_DBG_LEVEL,"  AttachedList has surface with handle=%ld dwCaps=%08lXh.  This surface has not yet been created!",
               pDDSLcl->lpSurfMore->dwSurfaceHandle, pDDSLcl->ddsCaps.dwCaps);

      // try lpLink
      curr = curr->lpLink;
      goto WORKAROUND_W2K_ATTACHED_SURFACE_LIST_ANOMALY;
    }

#ifdef WINNT
  } while (NULL != pDDSLcl);
#else
//PingZ 8/9/99 This is to work around what appears to be a bug in DX7 runtime in that
//pDDSLcl could be circling for ever.  To recreate this problem simply run and DX7 SDK sample
//app and go to full screen mode.
  } while ((NULL != pDDSLcl) && (pDDSLcl != pcsxd->lpDDSLcl));
#endif

  D3DPRINT(ENTRY_EXIT_DBG_LEVEL,"<< ddiCreateSurfaceEx");

#if ENABLE_LOG_FILE
  /* retro3dfx: a CreateSurfaceEx failure kills DX8 device creation silently
     (runtime destroys the fresh context and the app aborts its test). */
  if (DD_OK != pcsxd->ddRVal)
    retroLogForce(ppdev, "retro3dfx CSEX-FAIL: ddRVal=%08lXh caps=%08lXh hSurf=%ld\r\n",
                  (DWORD)pcsxd->ddRVal,
                  pcsxd->lpDDSLcl ? pcsxd->lpDDSLcl->ddsCaps.dwCaps : 0,
                  pcsxd->lpDDSLcl ? pcsxd->lpDDSLcl->lpSurfMore->dwSurfaceHandle : 0);
#endif

  return DDHAL_DRIVER_HANDLED;
} // ddiCreateSurfaceEx

/*----------------------------------------------------------------------
Function name: ddiDestroyDDLocal

Description:   DX7 Callback ddiDestroyDDLocal()

   ddiDestroyDDLocal destroys all the Direct3D surfaces previously created by
   ddiCreateSurfaceEx that belong to the same given local DirectDraw object.

   All Direct3D drivers must support ddiDestroyDDLocal.
   Direct3D calls ddiDestroyDDLocal when the application indicates that the
   Direct3D context is no longer required and it will be destroyed along with
   all surfaces associated to it. The association comes through the pointer to
   the local DirectDraw object. The driver must free any memory that the
   driver's ddiCreateSurfaceExDDK_ddiCreateSurfaceEx_GG callback allocated for
   each surface if necessary. The driver should not destroy the DirectDraw
   surfaces associated with these Direct3D surfaces; this is the application's
   responsibility.

   Parameters

        lpdddd
              Pointer to the DestoryLocalDD structure that contains the
              information required for the driver to destroy the surfaces.

              dwFlags
                    Currently unused
              pDDLcl
                    Pointer to the local Direct Draw object which serves as a
                    reference for all the D3D surfaces that have to be destroyed.
              ddRVal
                    Specifies the location in which the driver writes the return
                    value of ddiDestroyDDLocal. A return code of DD_OK indicates
                     success.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
               DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
ddiDestroyDDLocal( LPDDHAL_DESTROYDDLOCALDATA pdddd )
{
  NT9XDEVICEDATA  *ppdev;
  LPVOID          pDDLcl  = (LPVOID)pdddd->pDDLcl;
  HNDLLIST        *pHndlList;
  DWORD           i;


#ifdef WINNT
  ppdev = (NT9XDEVICEDATA *)pdddd->pDDLcl->lpGbl->dhpdev;
#else
  ppdev = (NT9XDEVICEDATA *)pdddd->pDDLcl->lpGbl->dwReserved3;
#endif

  D3DPRINT(ENTRY_EXIT_DBG_LEVEL,">> ddiDestroyDDLocal - ppdev %8lXh", ppdev);

  D3DPRINT(NORMAL_DBG_LEVEL, "pDDLcl = %8lXh", pDDLcl);

  pHndlList = GetHndlListPtr(ppdev, pDDLcl);

  if (NULL != pHndlList)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "pHndlList = %8lXh", pHndlList);

    // release any TXTRHNDL's
    if (NULL != pHndlList->ppTxtrHndlList)
    {
      // loop over the array and free any TXTRHNDL's that are still in use
      for (i = 1; i < (DWORD)pHndlList->ppTxtrHndlList[0]; i++)
        if (NULL != pHndlList->ppTxtrHndlList[i])
        {
          D3DPRINT(NORMAL_DBG_LEVEL, "Releasing TXTRHNDL %ld  TXTRHNDL ptr=%8lXh",
                   i, pHndlList->ppTxtrHndlList[i]);
          ReleaseTxtrHndl(ppdev, pHndlList, i);
        }

      // now release the memory for the array
      D3DPRINT(NORMAL_DBG_LEVEL,
               "Releasing pHndlList[%X]->ppTxtrHndlList[%X] for pDDLcl %X",
               pHndlList, pHndlList->ppTxtrHndlList, pDDLcl);
      D3DFREE(pHndlList->ppTxtrHndlList);
      pHndlList->ppTxtrHndlList = NULL;
    }

    // release any PALHNDL's
    if (NULL != pHndlList->ppPalHndlList)
    {
      // loop over the array and free any PALHNDL's that were allocated
      for (i = 1; i < (DWORD)pHndlList->ppPalHndlList[0]; i++)
        if (NULL != pHndlList->ppPalHndlList[i])
        {
          D3DPRINT(NORMAL_DBG_LEVEL, "Releasing PPALHNDL %8lXh (paletteHandle = %ld)",
                   pHndlList->ppPalHndlList[i], i);
          D3DFREE(pHndlList->ppPalHndlList[i]);
        }

      // now release the memory for the array
      D3DPRINT(NORMAL_DBG_LEVEL,
               "Releasing pHndlList[%X]->ppPalHndlList[%X] for pDDLcl %x",
               pHndlList, pHndlList->ppPalHndlList, pDDLcl);
      D3DFREE(pHndlList->ppPalHndlList);
      pHndlList->ppPalHndlList = NULL;
    }

    // now mark the HNDLLIST as unused
    if (NULL != pHndlList)
      ReleaseHndlList(ppdev, pHndlList);
  }

  pdddd->ddRVal = DD_OK;

  D3DPRINT(ENTRY_EXIT_DBG_LEVEL,"<< ddiDestroyDDLocal");

  return DDHAL_DRIVER_HANDLED;
} // ddiDestroyDDLocal

/*-------------------------------------------------------------------
Function Name:  GetHndlListPtr

Description:    finds the HndlList owned by pDDLcl.  If there
                isn't a HndlList owned by pDDLcl, then NULL is
                returned

Return:         HNDLLIST * or NULL
-------------------------------------------------------------------*/

HNDLLIST *
GetHndlListPtr(NT9XDEVICEDATA *ppdev, LPVOID pDDLcl)
{
  HNDLLIST  *pHndlList = (HNDLLIST *)&g_pHndlList;
  HNDLLIST  *pHL;


  D3DPRINT(10,">> GetHndlListPtr");
  ASSERTDD(NULL != pDDLcl, "GetHndlListPtr invalid pDDLcl");

  while (pHndlList->pNext)
  {
    pHL = pHndlList->pNext;
    if (pDDLcl == pHL->pDDLcl)
    {
      D3DPRINT(NORMAL_DBG_LEVEL,"found HndlList=%8lXh", pHL);
      return pHL;
    }
    pHndlList = pHndlList->pNext;
  }

  D3DPRINT(NORMAL_DBG_LEVEL, "no HndlList found");
  D3DPRINT(10,"<< GetHndlListPtr");

  return NULL;
}

/**************************************************************************
* S T A T I C   F U N C T I O N S
***************************************************************************/

/*-------------------------------------------------------------------
Function Name:  AllocHndlList

Description:    Allocates a HndlList and insert it at the head of
                a linked list

Return:         HNDLLIST * or NULL
-------------------------------------------------------------------*/

static HNDLLIST *
AllocHndlList(NT9XDEVICEDATA *ppdev, LPVOID pDDLcl)
{
  HNDLLIST  *pHL;


  D3DPRINT(10,">> AllocHndlList");

  pHL = (HNDLLIST *)D3DMALLOCZ(sizeof(HNDLLIST), 0);
  if (NULL != pHL)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "  allocated HNDLLIST = %8lXh", pHL);

    // insert new HNDLLIST at head of linked list
    if (NULL != g_pHndlList)
    {
      pHL->pNext = g_pHndlList;
      UPDATE_BLOCK_DATA(g_pHndlList, &pHL->pNext);
      g_pHndlList = pHL;
      UPDATE_BLOCK_DATA(pHL, &g_pHndlList);
      D3DPRINT(NORMAL_DBG_LEVEL, "  inserting at head of list, g_pHndlList=%8lXh, pHL->pNext=%8lXh",
               g_pHndlList, pHL->pNext);
    }
    else
    {
      pHL->pNext = NULL;
      g_pHndlList = pHL;
      UPDATE_BLOCK_DATA(pHL, &g_pHndlList);
      D3DPRINT(NORMAL_DBG_LEVEL, "  adding to empty list, g_pHndlList=%8lXh, pHL->pNext=%8lXh",
               g_pHndlList, pHL->pNext);
    }

    // if we allocated a HNDLLIST, then mark it used by this pDDLcl
    pHL->pDDLcl = pDDLcl;
  }
  else
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "  unable to allocate HNDLIST");
  }

#if defined(WINNT) && DBG
  // dump HNDLLIST linked list
  {
    HNDLLIST  *pHndlList2 = (HNDLLIST *)&g_pHndlList;

    D3DPRINT(NORMAL_DBG_LEVEL, "AllocHndlList - Current HNDLLIST list");
    while (pHndlList2->pNext)
    {
      D3DPRINT(NORMAL_DBG_LEVEL, "  pNext = %8lXh", pHndlList2->pNext);
      pHndlList2 = pHndlList2->pNext;
    }
  }
#endif

  D3DPRINT(10,"<< AllocTxtrHndl");

  return pHL;
}

/*-------------------------------------------------------------------
Function Name:  ReleaseHndlList

Description:    Remove a HNDLLIST from the linked list and free it

Return:         void
-------------------------------------------------------------------*/
static VOID
ReleaseHndlList(NT9XDEVICEDATA *ppdev, HNDLLIST *pHL)
{
  HNDLLIST  *pHndlList = (HNDLLIST *)&g_pHndlList;


  ASSERTDD(pHL != NULL, "can't free a NULL HNDLLIST");

  while (pHndlList->pNext)
  {
    if (pHL == pHndlList->pNext)
    {
      pHndlList->pNext = pHL->pNext;
#ifdef MEMCHECK
      UPDATE_BLOCK_DATA(pHL, 0);
      if (pHL->pNext)
        UPDATE_BLOCK_DATA(pHL->pNext, &pHndlList->pNext);
#endif

      D3DPRINT(NORMAL_DBG_LEVEL, "  after unlinking HNDLLIST=%8lXh, pHndlList=%8lXh, pHndlList->pNext=%8lXh",
               pHL, pHndlList, pHndlList->pNext);

      pHL->pNext = NULL;
      D3DFREE(pHL);

#if defined(WINNT) && DBG
      // dump HNDLLIST linked list
      {
        HNDLLIST  *pHndlList2 = (HNDLLIST *)&g_pHndlList;

        D3DPRINT(NORMAL_DBG_LEVEL, "ReleaseHndlList - Current HNDLLIST list");
        while (pHndlList2->pNext)
        {
          D3DPRINT(NORMAL_DBG_LEVEL, "  pNext = %8lXh", pHndlList2->pNext);
          pHndlList2 = pHndlList2->pNext;
        }
      }
#endif

      return;
    }
    pHndlList = pHndlList->pNext;
  }

} // ReleaseHndlList

/*-------------------------------------------------------------------
Function Name:  GetTxtrHndl

Description:    Find the index of the TXTRHNDL associated to this
                pDDLcl-Handle pair

Return:         0 or index into pHndlList->ppTxtrHndlList
-------------------------------------------------------------------*/

static DWORD
GetTxtrHndl(NT9XDEVICEDATA *ppdev,
            LPVOID pDDLcl,
            HNDLLIST *pHndlList,
            DWORD dwSurfaceHandle)
{
  D3DPRINT(10,">> GetTxtrHndl");
  ASSERTDD(pHndlList->pDDLcl == pDDLcl, "GetTxtrHndl invalid pDDLcl");

  // see if ppTxtrHndlList has an array allocated for it
  // and if so how long it is
  if ((NULL != pHndlList->ppTxtrHndlList) &&
      ((DWORD)pHndlList->ppTxtrHndlList[0] > dwSurfaceHandle))
  {
    if (NULL != pHndlList->ppTxtrHndlList[dwSurfaceHandle])
    {
      D3DPRINT(NORMAL_DBG_LEVEL,"found TxtrHndl=%lXh for surfHandle=%ld",
               pHndlList->ppTxtrHndlList[dwSurfaceHandle], dwSurfaceHandle);
      D3DPRINT(10,"<< GetTxtrHndl");

      pHndlList->ppTxtrHndlList[dwSurfaceHandle]->flags = HandleInUse;

      return dwSurfaceHandle;
    }
  }

  D3DPRINT(NORMAL_DBG_LEVEL, "no TxtrHndl found");
  D3DPRINT(10,"<< GetTxtrHndl");

  return 0;
} // GetTxtrHndl

/*-------------------------------------------------------------------
Function Name:  AllocTxtrHndl

Description:    Allocates a TXTRHNDL in the ppTxtrHndlList array.
                If the array isn't large enough, a new, larger array is
                allocated and the old array is copied into it.

Return:         0 or index into ppHndlList->ppTxtrHndlList
-------------------------------------------------------------------*/

static DWORD
AllocTxtrHndl(NT9XDEVICEDATA *ppdev, LPVOID pDDLcl, HNDLLIST *pHndlList, DWORD dwSurfaceHandle)
{
  DWORD     dwTxtrHndl;
  TXTRHNDL  *pTxtrHndl;


  D3DPRINT(10,">> AllocTxtrHndl");

  ASSERTDD(NULL != pDDLcl && NULL != pHndlList, "AllocTxtrHndl invalid input");
  ASSERTDD(pHndlList->pDDLcl == pDDLcl, "AllocTxtrHndl invalid pDDLcl");

  // if we don't have a ppTxtHndlList or it's not big enough then
  // resize the current one
  if (NULL == pHndlList->ppTxtrHndlList ||
      dwSurfaceHandle > (DWORD)pHndlList->ppTxtrHndlList[0])
  {
    // dwSurfaceHandle numbers are going to be ordinal numbers starting
    // at one, so we use this number to figure out a "good" size for
    // our new list.
    // we need to account for using index zero as a count of how many elements are in
    // the ppTxtrHndlList array, so add one to dwSurfaceHandle
    DWORD newsize = (((dwSurfaceHandle + 1) + (LISTGROWSIZE - 1)) / LISTGROWSIZE) * LISTGROWSIZE;
    TXTRHNDL **newlist = (TXTRHNDL **)D3DMALLOCZ(sizeof(TXTRHNDL *) * newsize, 0);
    D3DPRINT(NORMAL_DBG_LEVEL,"Growing pDDLcl=%X's ppTxtrHndlList[%X] size to %08lXh",
             pDDLcl, newlist, newsize);

    if (NULL == newlist)
    {
      D3DPRINT(0, "AllocTxtrHndl failed to increase pHndlList->ppTxtrHndlList[%8lXh]",
               pHndlList);
      return 0;
    }

    // if we had a valid TXTRHNDL list,
    // copy it to the newlist and free the memory allocated before
    if (NULL != pHndlList->ppTxtrHndlList)
    {
      // copy counter in element zero plus all TXTRHNDL *'s to new array
      memcpy(newlist, pHndlList->ppTxtrHndlList,
             ((DWORD)pHndlList->ppTxtrHndlList[0] + 1) * sizeof(TXTRHNDL *));
#ifdef MEMCHECK
      {
        DWORD i;

        for (i = 1; i < (DWORD)pHndlList->ppTxtrHndlList[0]; i++)
        {
          if (NULL != pHndlList->ppTxtrHndlList[i])
            UPDATE_BLOCK_DATA(pHndlList->ppTxtrHndlList[i], &newlist[i]);
        }
      }
#endif
      D3DFREE(pHndlList->ppTxtrHndlList);
      D3DPRINT(NORMAL_DBG_LEVEL,"Freeing pDDLcl=%X's old ppTxtrHndlList[%X]",
               pDDLcl, pHndlList->ppTxtrHndlList);
    }

    pHndlList->ppTxtrHndlList = newlist;
    UPDATE_BLOCK_DATA(newlist, &pHndlList->ppTxtrHndlList);
    // store size in ppTxtrHndlList[0]
    // since we're using element zero to hold the array size
    // we only have newsize-1 entries to store TXTRHNDL elements in
    (DWORD)pHndlList->ppTxtrHndlList[0] = newsize - 1;
  }

  // If we don't have a TXTRHNDL hanging from this TXTRHNDL list
  // element we have to create one.
  if (NULL == pHndlList->ppTxtrHndlList[dwSurfaceHandle])
  {
    // now allocate a TXTRHNDL
    pTxtrHndl = (TXTRHNDL *)D3DMALLOCZ(sizeof(TXTRHNDL), 0);

    if (NULL == pTxtrHndl)
    {
      D3DPRINT(0, "AllocTxtrHndl out of memory, failed to alloc TXTRHNDL");
      return 0;
    }

    // mark it in use
    pTxtrHndl->flags = HandleInUse;

    // store the TXTRHNDL in the pHndlList array in the dwSurfaceHandle element
    pHndlList->ppTxtrHndlList[dwSurfaceHandle] = pTxtrHndl;
    UPDATE_BLOCK_DATA(pTxtrHndl, &pHndlList->ppTxtrHndlList[dwSurfaceHandle]);
  }
  else
  {
    pTxtrHndl = pHndlList->ppTxtrHndlList[dwSurfaceHandle];
    // mark it in use
    pTxtrHndl->flags = HandleInUse;
  }

  dwTxtrHndl = dwSurfaceHandle;

  D3DPRINT(NORMAL_DBG_LEVEL,"Set pDDLcl=%8Xlh Handle=%ld dwTxtrHndl = %ld, pTxtrHndl=%8lXh",
           pDDLcl, dwSurfaceHandle, dwTxtrHndl, pTxtrHndl);

  D3DPRINT(10,"<< AllocTxtrHndl");

  return dwTxtrHndl;
} // AllocTxtrHndl

/*-------------------------------------------------------------------
Function Name:  ReleaseTxtrHndl

Description:    Release a previously allocated TXTRHNDL

Return:         void
-------------------------------------------------------------------*/

VOID
ReleaseTxtrHndl(NT9XDEVICEDATA *ppdev, HNDLLIST *pHndlList, DWORD dwTxtrHndl)
{
  TXTRHNDL  *pTxtrHndl = pHndlList->ppTxtrHndlList[dwTxtrHndl];

#ifdef WINNT
  // to fix potential access violation issues with NT Stress
  // verify we have a TXTRHNDL for this texture before dereferencing it
  //
  // no additional check for NULL TXTRHNDL needed here since this function
  // won't be called if the TXTRHNDL for dwTxtrHndl is NULL
#endif

  if (NULL != pTxtrHndl)
  {
    memset(pTxtrHndl, 0, sizeof(TXTRHNDL));
    D3DFREE(pTxtrHndl);
    pHndlList->ppTxtrHndlList[dwTxtrHndl] = NULL;
  }
}

#if defined(TnL_HAL) && defined(VERT_BUFF)

/*-------------------------------------------------------------------
Function Name:  CanCreateExecuteBuffer32

Description:    

Return:         
-------------------------------------------------------------------*/

/**********************************************************************************
There's a bug in the DDK that we work around here.  The 
LPDDHAL_CANCREATESURFACEDATA struct is described in ddrawi.h:

typedef struct _DDHAL_CANCREATESURFACEDATA
{
    LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
    LPDDSURFACEDESC             lpDDSurfaceDesc;    // description of surface being created
    DWORD                       bIsDifferentPixelFormat;// pixel format differs from primary surface
    HRESULT                     ddRVal;         // return value
    LPDDHAL_CANCREATESURFACE    CanCreateSurface;   // PRIVATE: ptr to callback
} DDHAL_CANCREATESURFACEDATA;
typedef struct _DDHAL_CANCREATESURFACEDATA FAR *LPDDHAL_CANCREATESURFACEDATA;

The type of the 2nd parameter should actually be a LPDDSURFACEDESC2.  The DX 
runtime  apperently casts this themselves but didn't tell us.  I verified this 
is the actual structure because the size parameter of the struct is 7Ch bytes.  

The DX docs are also wrong ...
**********************************************************************************/
DWORD __stdcall CanCreateExecuteBuffer32( LPDDHAL_CANCREATESURFACEDATA lpd)
{
  LPDDSURFACEDESC2 lpddsd;
  DWORD dwCaps, dwCaps2;

  DPF("in CanCreateExecuteBuffer32");

  lpddsd  = (LPDDSURFACEDESC2) lpd->lpDDSurfaceDesc;
  dwCaps  = lpddsd->ddsCaps.dwCaps;
  dwCaps2 = lpddsd->ddsCaps.dwCaps2;

  // If this is a write-only vertex buffer 
  // we'll give it a shot!
  if ((dwCaps & DDSCAPS_WRITEONLY) &&		// Check for write only
      (dwCaps2 & DDSCAPS2_VERTEXBUFFER))	// Check for an explict vertex buffer
    lpd->ddRVal = DD_OK;
  else
    lpd->ddRVal = DDERR_UNSUPPORTED;

  return DDHAL_DRIVER_HANDLED;
}

/*-------------------------------------------------------------------
Function Name:  CreateExecuteBuffer32

Description:    

Return:         
-------------------------------------------------------------------*/

DWORD __stdcall CreateExecuteBuffer32( LPDDHAL_CREATESURFACEDATA  pcsd)
{
  LPDDRAWI_DDRAWSURFACE_LCL FAR *lplpSList;
  LPDDRAWI_DDRAWSURFACE_LCL lpSurf;
  LPDDRAWI_DDRAWSURFACE_GBL psurf_gbl;
  int i;
  DWORD dwCaps, dwCaps2, dwSize; 
  LPVBSURFACEDATA surfaceData;

  DD_ENTRY_SETUP(pcsd->lpDD);

  DPF("in CreateExecuteBuffer32");

  lplpSList = pcsd->lplpSList;
  pcsd->ddRVal = DD_OK;

  for(i = 0;i < (int)pcsd->dwSCnt; i++)
  {
    lpSurf = lplpSList[i];
    psurf_gbl = lpSurf->lpGbl;

    dwCaps = lpSurf->ddsCaps.dwCaps;
    dwCaps2 = lpSurf->lpSurfMore->ddsCapsEx.dwCaps2;

    D3DPRINT(0,",CreateExecBuff,%ld,%ld,%08lX,%08lX", lpSurf->lpGbl->dwLinearSize, i, dwCaps, dwCaps2);

    // BobJ 11/15/99 - For the Software T&L HAL we want to take over the 
    // Vertex Buffer creation process.
    if(!(dwCaps2 & (DDSCAPS2_VERTEXBUFFER)))
    {
      // The VERTEXBUFFER flag is not present in dwCaps2
      // It's either an implicit VB or a Command Buffer
      // In any case, punt the creation back to DirectDraw
      // Since we don't really care about these buffers now.
      psurf_gbl->dwReserved1 = 0;
      pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
      return DDHAL_DRIVER_HANDLED;
    }
    else
    {
      // It's a real Vertex buffer!  Let's take over the 
      // Allcation and Management of this VB.

      //-------------------------------------------------
      // First things first.  We need a litle data
      // structure that descibes our use of this Vertex 
      // Buffer Allocate and Zero out our VBSURFACEDATA 
      // data struture
      //-------------------------------------------------
      surfaceData = (LPVBSURFACEDATA) D3DMALLOCZ(sizeof(VBSURFACEDATA), 0);

      if(!surfaceData)
      {
        pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
        return DDHAL_DRIVER_HANDLED;
      }
      // Assign this structure pointer to the GBL dwReserved1 
      // field so we can find it quickly in DP2
      psurf_gbl->dwReserved1 = (DWORD)surfaceData;
      UPDATE_BLOCK_DATA(surfaceData, &psurf_gbl->dwReserved1);

      //-------------------------------------------------
      // Next, we need to Allocate the real Source Vertex
      // Buffer that DD is asking for
      // Calculate size for Source VB and Allocate it
      //-------------------------------------------------
      dwSize = lpSurf->lpGbl->dwLinearSize + (31 + 288); 
                      // Add an extra 288 bytes just to make sure that a swizzle on an odd
                      // number of vertices has room to swizzle dummy values on the extra
                      // allocated SOA group.
      surfaceData->pSrcAllocAddr = (LPVOID) D3DMALLOC(dwSize, &surfaceData->pSrcAllocAddr);
      if (!surfaceData->pSrcAllocAddr)
      {
        D3DFREE(surfaceData);
        psurf_gbl->dwReserved1 = 0;
        pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
        return DDHAL_DRIVER_HANDLED;
      }

      // Looks like we have a valid Source VB memory allocation
      // Need to fillout the rest of the VBSURFACEDATA
      surfaceData->pSrcAlignAddr = (LPVOID)(((ULONG_PTR)surfaceData->pSrcAllocAddr + 31 ) & ~31);
      surfaceData->dwSrcAllocSize = dwSize;
      surfaceData->dwSrcAlignSize = dwSize - ((DWORD)surfaceData->pSrcAlignAddr - (DWORD)surfaceData->pSrcAllocAddr);
      surfaceData->dwSrcFlags |= VBSURF_JUSTCREATED;    // Mark the VB as Just Created!

      // Kind of silly to check this cause we reject non-WO
	  // buffers in CanCreate, but let's be sure
      if((dwCaps & DDSCAPS_WRITEONLY))
      {
         // If this VB is marked as WRITEONLY
         // Let DP2 know about it.  It is our
         // Favorite kind of Vertex Buffer!
         surfaceData->dwSrcFlags |= VBSURF_WRITEONLY;
      }
      
      // Let's fill out everything MS wants us to
	  // about the allocation of this surface
      psurf_gbl->fpVidMem = (FLATPTR) surfaceData->pSrcAlignAddr;			// aligned allocation addr
      psurf_gbl->lPitch = lpSurf->lpGbl->dwLinearSize;   					// size in bytes
      lpSurf->lpGbl->dwBlockSizeX = lpSurf->lpGbl->dwLinearSize;		    // size in bytes (again)
      lpSurf->lpGbl->dwBlockSizeY = 1;										// Height of 1
      lpSurf->lpGbl->dwGlobalFlags = DDRAWISURFGBL_SYSMEMEXECUTEBUFFER;     // Tell DD that we're in System Memory!
	  pcsd->lpDDSurfaceDesc->lPitch = lpSurf->lpGbl->dwLinearSize;          // size in bytes (for the 3rd time)

    }
  }

  return DDHAL_DRIVER_HANDLED;
}

/*-------------------------------------------------------------------
Function Name:  DestroyExecuteBuffer32

Description:    

Return:         
-------------------------------------------------------------------*/

DWORD __stdcall DestroyExecuteBuffer32( LPDDHAL_DESTROYSURFACEDATA pdsd)
{
  LPDDRAWI_DDRAWSURFACE_LCL psurf;
  LPDDRAWI_DDRAWSURFACE_GBL psurf_gbl;
  LPVBSURFACEDATA   surfaceData;

  DD_ENTRY_SETUP(pdsd->lpDD);

  psurf = pdsd->lpDDSurface;
  psurf_gbl = psurf->lpGbl;

  DPF("in DestroyExecuteBuffer32");
  pdsd->ddRVal = DD_OK;

  if ((DDHAL_PLEASEALLOC_BLOCKSIZE == psurf_gbl->fpVidMem ) || 
      (NULL == (LPVOID)psurf_gbl->fpVidMem))
    return DDHAL_DRIVER_NOTHANDLED;

  surfaceData = (LPVBSURFACEDATA) psurf_gbl->dwReserved1;
  if(surfaceData != (LPVBSURFACEDATA) NULL)
  {
    D3DPRINT(32, "DestroyExecBuff32 Opt = %08lX, Src = %08lX, surfData= %08lX", 
                                 (DWORD)surfaceData->pOptAllocAddr, (DWORD)surfaceData->pSrcAllocAddr, (DWORD)surfaceData);
    if( NULL != (LPVOID)surfaceData->pOptAllocAddr)
       D3DFREE(surfaceData->pOptAllocAddr);

    if( NULL != (LPVOID)surfaceData->pSrcAllocAddr)
       D3DFREE(surfaceData->pSrcAllocAddr);

    memset((void *)surfaceData, 0, sizeof(VBSURFACEDATA));

    D3DFREE((void*)surfaceData);
    psurf_gbl->dwReserved1 = 0;
  }

  psurf_gbl->fpVidMem=(FLATPTR)NULL;
  return DDHAL_DRIVER_HANDLED;
}

/*-------------------------------------------------------------------
Function Name:  LockExecuteBuffer32

Description:    

Return:         
-------------------------------------------------------------------*/

DWORD __stdcall LockExecuteBuffer32( LPDDHAL_LOCKDATA lpd)
{
  LPVBSURFACEDATA   surfaceData;

  DD_ENTRY_SETUP(lpd->lpDD);

  surfaceData = (LPVBSURFACEDATA) lpd->lpDDSurface->lpGbl->dwReserved1;
  /* Avoid (non-texture) surfaces which have been invalidated by a mode change. - CGW */

  DPF("in LockExecuteBuffer32");

  // For some weird reason, we're being asked to 
  // lock surface that we know nothing about
  // We'll just return the addr anyway
  if (surfaceData == NULL)
  {
    lpd->lpSurfData = (LPVOID)lpd->lpDDSurface->lpGbl->fpVidMem;
    lpd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
  }

  D3DPRINT(32, "LockExecBuf32 fpVidMem = %08lX", lpd->lpDDSurface->lpGbl->fpVidMem );

  // Let's report back to the memory address and set
  // some surface management flags
  lpd->lpSurfData = (LPVOID)lpd->lpDDSurface->lpGbl->fpVidMem;
  surfaceData->dwSrcFlags |= VBSURF_ACCESSED;

  lpd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
}

/*-------------------------------------------------------------------
Function Name:  UnlockExecuteBuffer32

Description:    

Return:         
-------------------------------------------------------------------*/

DWORD __stdcall UnlockExecuteBuffer32( LPDDHAL_UNLOCKDATA puld)
{
  LPVBSURFACEDATA   surfaceData;
  DD_ENTRY_SETUP(puld->lpDD);

  surfaceData = (LPVBSURFACEDATA) puld->lpDDSurface->lpGbl->dwReserved1;

  DPF("in UnlockExecuteBuffer32");

  if (surfaceData == NULL)
  {
    puld->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
  }
  
  D3DPRINT(32, "UnlockExecBuf32 fpVidMem = %08lX", puld->lpDDSurface->lpGbl->fpVidMem );

  // Let's set some surface management flags
  surfaceData->dwSrcFlags |= VBSURF_ACCESSED;
  surfaceData->dwUnlockCounter++;

  puld->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
}

#endif  //VERT_BUFF

#endif


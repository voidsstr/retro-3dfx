/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** File name: D3contxt.c
**
** Description: Implements context initialization and destruction for D3D.
**
** $Revision: 16$
** $Date: 10/31/00 2:00:42 AM$
**
** $Log: 
**  16   3dfx      1.11.3.3    10/31/00 Johnny Trainor  Initialise new elements of
**       our RC structure.
**  15   3dfx      1.11.3.2    10/25/00 Johnny Trainor  Updated so we no longer use
**       surface local pointers.
**  14   3dfx      1.11.3.1    10/13/00 Johnny Trainor  Initialize the DX interface
**       version and set D3DRENDERSTATE_TEXTUREPERSPECTIVE to true for DX8 builds.
**  13   3dfx      1.11.3.0    10/11/00 Brent           Forced check in to enforce
**       branching.
**  12   3dfx      1.11        04/06/00 Christopher Wilcox Removed error code for
**       zbuffer and render targer depth mismatch.
**  11   3dfx      1.10        03/14/00 Russ Lind       in ddiContextCreate, if the
**       zbuffer and render target have different bit depths increment
**       _D3(dwRenderTargZBuffMismatchCnt), if the render target and zbuffer have
**       the same bit depth set _D3(dwRenderTargZBuffMismatchCnt) = 0
**  10   3dfx      1.9         03/11/00 Bob Seitsinger  Debug statements to assist
**       in buffer allocation/deallocation debugging.
**  9    3dfx      1.8         03/10/00 Russ Lind       plug memory leak in
**       ddiContextCreate
**  8    3dfx      1.7         03/01/00 Christopher Wilcox Fixed problems with
**       16bpp render target and 32bpp zbuffer and vice versa.  Resolves PRS 13027
**       and 13069.
**  7    3dfx      1.6         02/23/00 Sam Hanna       Fixed PRS 12728  (H5TOT
**       2SLI: Hidden & Dangerous, black screen with sound during intro scenes) by
**       initializing the clipping values when a new context handle is created for
**       the surface to be used as the rendering target.
** 
**  6    3dfx      1.5         12/13/99 Scott Kephart   Big T&L Update:
**       1. Improved T&L profiling code
**       2. Optimizations to scalar transformation and lighting code
**       3. Changes to the vertex buffer code to allow functionality under Windows
**       2000.
**  5    3dfx      1.4         12/03/99 Christopher Wilcox Updated surface access
**       method for DirectX 7.
** 
**  4    3dfx      1.3         11/10/99 Scott Kephart   Added profiling changes for
**       T&L
**  3    3dfx      1.2         10/26/99 Scott Kephart   Added initial support for
**       software T&L HAL
**  2    3dfx      1.1         09/30/99 Christopher Wilcox Fixes for DirectX 7
**       multi-buffering problem (PRS #8782)
** 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 15    8/30/99 10:00a Russ
** yet another check for a NULL pointer to fix another NT Stress access
** violation
**
** 14    8/23/99 11:27a Russ
** for DX7, additional error checking in ddiContextCreate before returning
** a valid RC pointer.  Fixes an access violation running NT Stress.
**
** 13    8/20/99 12:37p Russ
** added g_pContexts as a global var for W2K
**
** 12    8/12/99 2:35p Russ
** change to a debug output message
**
** 11    8/11/99 3:46p Pzheng
** fixed a compiler error for Win9x build
**
** 10    8/10/99 10:35a Russ
** for DX7, changes to replace use of TXTRHNDL array with dynamic
** allocation
**
** 9     8/10/99 10:49a Pzheng
** Check newcontext->lpDDSZ instead of lpcc->lpDDSZ
**
** 8     8/06/99 6:08p Pzheng
** Corrected a DX7 incompatibility problem
**
** 7     8/05/99 4:10p Russ
** a couple NT debug output changes
**
** 6     8/02/99 12:30p Russ
** for W2K, restore ASSERTDD's and D3DPRINT that were disabled last week
**
** 5     7/28/99 11:58p Bseitsin
** Changes to enable DX7 for W9x.
**
** 4     7/12/99 1:05p Russ
** for DX7, initialize the state block elements in the RC struct in
** ddiContextCreate
** release any state blocks in ddiContextDestroy
**
** 3     6/21/99 5:20p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
**
** 2     6/11/99 12:37p Russ
** initial changes for DX7
**
** 1     6/02/99 6:40a Michael
** Branch from H3
**
** 25    5/30/99 5:35p Edwin
** Remove ifdef MM, multi-monitor support is always enabled.
**
** 24    5/24/99 5:04p Bseitsin
** Removal of Antialiasing code.
**
** 23    4/22/99 10:18a Cshaw
** Put back the Speed Buster fix (seems like the fix either never made it
** in or was wiped out somehow).
**
** 22    3/10/99 3:27p Cshaw
** Fix for PRS #4335 (SpeedBusters).  Init ChromaRange register.
**
** 21    1/29/99 10:48a Cshaw
** Added unified headers.
**
** 20    12/16/98 6:31a Russ
** fix for corruption when resizing tunnel or twist window on NT5
**
** 19    12/09/98 7:08p Adrians
** Fix for triple buffering in WinBench99.
**
** 18    12/09/98 7:20a Russ
** NT5 D3D changes for Banshee
**
** 17    12/02/98 8:24a Martin
** Turn off anti-aliasing by default and rename some variables for
** super-sampling AA.
**
** 16    11/22/98 8:55p Andrew
** Changes to support multi-monitor
**
** 15    11/13/98 2:52p Miriam
** Support for DX6 triangle flavor & texture flavor tracing. Just compile
** with debug & fp=1 or tp=1.
**
** 14    9/25/98 6:44p Adrians
** Addtional instrumentation changes.
**
** 13    8/28/98 12:37p Martin
** Do super-sampling AA when the registry key is set to 2.  Do edge AA if
** registry key == 1 OR we are not running at the 2 magical resolutions of
** 640x480 or 800x600.
**
** No more conditional compilation of AA.
**
** 12    8/23/98 12:49a Adrians
** Added support for AA SuperSampling.
**
** 11    7/24/98 1:37p Hohn
**
** 10    5/06/98 6:07p Adrians
** Changes for DX6 into DX5 driver.
**
** 2     5/01/98 4:11p Adrians
** Compile options for dx5 and dx6.
** Removed redundent returns.
**
**
**
** 1     4/29/98 6:31p Adrians
** Created
**
** 9     3/27/98 11:15a Adrians
** Code added for triangle profiling.
 *
 * 8     11/23/97 3:56p Suninn
 * replay _d3Global with _D3 & D3G macros
 *
 * 7     11/09/97 2:49p Adrians
 * Single pk1's use an increment of 0.
 * Code added to find the triangle flavours used by apps.
 * Optimisation of triangle flavours.
 * Support for strips and fans in execute buffers.
 * Bug fix to wrapU and wrapV modes.
 *
 *
 * 6     10/14/97 11:22a Adrians
 * Added Miriam's 4M texture support.  Added build environment for H3.
 *
 * 5     10/09/97 10:23a Adrians
 * Added some SST1 code changes.
 *
 * 4     10/02/97 8:38p Adrians
 * Include init code into build. Enable Write Combining.  Inline system
 * functions.  Change optimisations.  Some code tidy up.
 *
 * 3     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/
#include "precomp.h"

#ifndef WINNT
#include "d3dhal.h"
#include "hw.h"
#include "d3global.h"
#include "d3txtr.h"
#define  FX_DEFINE_MACROS
#include "fxglobal.h"
#include "fifomgr.h"
#include "d3contxt.h"
#endif

#include "dxins.h"

#if RC_LINKED_LIST && defined(WINNT)
// due to calling sequence on W2K, we must have the head of
// the linked list as a global variable
RC *g_pContexts;
#endif

#if defined(LEGACY_APPS) && !defined(NT)

static BOOL IsLegacyApp();
static VOID GetProcessFileName(LPSTR, DWORD);

typedef struct tag_LegacyAppID
{
  PSTR  pszProcFileName;    // legacy app's module filename
  DWORD appFlag;
} LEGACYAPPID;

static LEGACYAPPID LegacyApps[] =
{
  { "BEND3DIM.EXE", LEGACYAPP1 },
};

/*-------------------------------------------------------------------
Function Name:  GetProcessFileName
Description:    Gets the process' file name.
Information:    static VOID GetProcessFileName(LPSTR lpBuf, DWORD dwSize)
Return:         VOID
                lpBuf - fills in the process name string at this address.
                        ('\0' indicates we did not get the process name)
				
				
				
-------------------------------------------------------------------*/
static VOID GetProcessFileName(LPSTR lpBuf, DWORD dwSize)
{
  HANDLE hFile;
  DWORD  i;

  // GetModuleFileName returns length of filename (exclude null terminated
  // char) if successful, else returns zero

  hFile = GetModuleHandle(NULL);
  if ( (i = GetModuleFileName(hFile, lpBuf, dwSize)) )
  {
    // locate first char of filename, excluding pathname

    i--;                    // index starts from zero, points at last char
    while ( (i >=0) && (lpBuf[i] != '\\') )
    {
        i--;
    }
    i++;                    // lpBuf[0] points to first char of filename
    strcpy(lpBuf, &lpBuf[i]);
  }
  else {
    lpBuf[0] = '\0';        // indicate we did not get the process name
  }
}

/*-------------------------------------------------------------------
Function Name:  IsLegacyApp
Description:    Checks to see if the Process File Name is a legacy app.
Information:    static BOOL IsLegacyApp()
Return:         BOOL  FALSE - if app is not in legacy app table.
                pLegacyApp->appFlag if app is in legacy app table.
				
-------------------------------------------------------------------*/
static BOOL IsLegacyApp()
{
  CHAR  szBuffer[MAX_PATH];
  LEGACYAPPID *pLegacyApp;

  GetProcessFileName(szBuffer, sizeof(szBuffer));
  if ('\0' == szBuffer[0])
    return FALSE;

  for (pLegacyApp = &LegacyApps[0]; pLegacyApp < &LegacyApps[sizeof(LegacyApps)/sizeof(LegacyApps[0])];
       pLegacyApp++)
  {
    if ( !strcmp(pLegacyApp->pszProcFileName, szBuffer) )
      return pLegacyApp->appFlag;
  }

  return FALSE;
}

#endif    // LEGACY_APPS && !NT


/*-------------------------------------------------------------------
Function Name:  ddiContextCreate
Description:    Entry point for ContextCreate function.
Information:    DWORD __stdcall ddiContextCreate(LPD3DHAL_CONTEXTCREATEDATA pccd)
Return:         DWORD DDHAL_DRIVER_HANDLED
                pccd->ddrval =
                                D3DHAL_OUTOFCONTEXTS
                            or	DD_OK
-------------------------------------------------------------------*/

DWORD __stdcall ddiContextCreate(LPD3DHAL_CONTEXTCREATEDATA pccd)
{
  RC  *newContext;

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
  DWORD dwDXInterface;
#endif

#if (DIRECT3D_VERSION < 0x0700) || (DX < 7)
  LPDDRAWI_DDRAWSURFACE_INT interfaceDDSZ;
#endif
  NT9XDEVICEDATA  *ppdev;

  D3D_ENTRY( "ddiContextCreate" );
  INS_ENTRY( INSC_D3DCONTEXTCREATE );

#ifdef WINNT
  #if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    // what da hell?  DX7 stores the ppdev in a new location
    ppdev = (NT9XDEVICEDATA *)pccd->lpDDLcl->lpGbl->dhpdev;
  #else
    ppdev = (NT9XDEVICEDATA *)pccd->lpDDGbl->dhpdev;
  #endif
#else
  #if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    ppdev = (GLOBALDATA *)pccd->lpDDLcl->lpGbl->dwReserved3;
  #else
    ppdev = (GLOBALDATA *)pccd->lpDDGbl->dwReserved3;
  #endif
#endif

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
  // Save the DX API interface value (0=DX3, 1=DX5, 2=DX6, 3=DX7, 4=DX8)
  dwDXInterface = (DWORD) pccd->dwhContext;
#endif

  //--------------------------------------------------------------------------
  // Allocate a new context handle:
  // This callback is invoked when a new surface is to be used as a
  // rendering target. The context handle returned will be used whenever rendering
  // to this surface is to be performed.
  //--------------------------------------------------------------------------
  pccd->dwhContext = CONTEXT_ALLOC(ppdev);
  if (pccd->dwhContext == CONTEXT_INVALID)
  {
#if ENABLE_LOG_FILE
    retroLogForce(ppdev, "retro3dfx CTX-CREATE FAIL: OUTOFCONTEXTS (leak across warm reruns?)\r\n");
#endif
    pccd->ddrval = D3DHAL_OUTOFCONTEXTS;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
  newContext = CONTEXT_PTR(pccd->dwhContext);
#if ENABLE_LOG_FILE
  {
    /* retro3dfx: running create/destroy balance for the warm-rerun leak hunt */
    extern LONG g_retroCtxLive;
    g_retroCtxLive++;
    retroLogForce(ppdev, "retro3dfx CTX-CREATE: h=%08lXh live=%ld 3dCnt=%ld\r\n",
                  pccd->dwhContext, g_retroCtxLive, _FF(dd3DSurfaceCount));
  }
#endif

  D3DPRINT( 255,"ddiContextCreate, pccd->lpDDGbl =%08lx, pccd->lpDDS =%08lx",
                                pccd->lpDDGbl, pccd->lpDDS );
  D3DPRINT( 255,"                  pccd->dwhContext =%08lx, pccd->lpDDSZ= %08lx",
                                pccd->dwhContext, pccd->lpDDSZ );
#ifdef WINNT
  D3DPRINT(D3DDBGLVL, "context=%lXh, lpDDS=%lXh, lpDDSZ=%lXh, ppdev=%8lXh",
           pccd->dwhContext, pccd->lpDDS, pccd->lpDDSZ, ppdev);
#endif

  //---------------------------------------------------------------------------
  // Init the new Rendering context.
  // lpDDGbl is the global data for DDRAW object.
  // lpDDS is the surface data which contains pointer to surface global data.
  //---------------------------------------------------------------------------
#ifdef WINNT
  newContext->ppdev =               ppdev;
#else
  #if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  newContext->lpDDGbl =             (LPDDRAWI_DIRECTDRAW_GBL)pccd->lpDDLcl->lpGbl;
  #else
  newContext->lpDDGbl =             (LPDDRAWI_DIRECTDRAW_GBL)pccd->lpDDGbl;
  #endif
  newContext->ppdev =               (GLOBALDATA *)newContext->lpDDGbl->dwReserved3;
#endif
  initNewRC(newContext);
  STATESET_INIT(newContext->overrides);
  newContext->dwVertexType =        D3DFVF_TLVERTEX;

#ifdef WINNT									
  newContext->pid   =               pccd->dwPID;
#else
  #if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    newContext->pid    =            pccd->dwPID;
  #else
    newContext->interfaceDDS =      (LPDDRAWI_DDRAWSURFACE_INT) pccd->lpDDS;
    interfaceDDSZ = (LPDDRAWI_DDRAWSURFACE_INT) pccd->lpDDSZ;
    newContext->lpDDS =             newContext->interfaceDDS->lpLcl;
    newContext->pid =               pccd->dwPID;

    if (interfaceDDSZ)
      newContext->lpDDSZ =          interfaceDDSZ->lpLcl; // Z surface
    else
      newContext->lpDDSZ =          NULL;
  #endif

    // Setup the DX interface version creating this context (DX 8)
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
    newContext->dwDXInterface = dwDXInterface;
#endif

#if defined(LEGACY_APPS)
  _D3( legacyApp ) = IsLegacyApp();
#endif // LEGACY_APPS

#endif //WINNT

	// Initialize the clipping values for this surface. It is necessary to
	// set them at this point until the viewport clipping values are invoked.
	// This fixes PRS 12728 - H5TOT 2SLI: Hidden & Dangerous, black screen with sound during intro scenes
	newContext->sst.clipLeftRight = (DWORD) pccd->lpDDSLcl->lpGbl->wWidth;
	newContext->sst.clipBottomTop = (DWORD) pccd->lpDDSLcl->lpGbl->wHeight;

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  {
    HNDLLIST  *pHndlList;
    DWORD     i;

    pHndlList = GetHndlListPtr(ppdev, pccd->lpDDLcl);
    // IGX-NVH  8.25.99
    // hit this assert running NT Stress
#ifdef WINNT
    ASSERTDD(NULL != pHndlList, "no HNDLLIST allocated for this pDDLcl!");
#endif

    newContext->pHndlList =         pHndlList;
    newContext->pDDLcl =            pccd->lpDDLcl;
    newContext->DDSHndl =           pccd->lpDDSLcl->lpSurfMore->dwSurfaceHandle;
    if ((NULL == newContext->pHndlList)                 ||  // IGX-NVH 08.25.99  Got an AV because GetHndlListPtr() returned a NULL
        (NULL == newContext->pHndlList->ppTxtrHndlList) ||
        (NULL == newContext->pHndlList->ppTxtrHndlList[newContext->DDSHndl]))
    {
      D3DPRINT(0, "ddiContextCreate - txtrHndl is NULL for the render target");
      D3DPRINT(0, "                   failing context creation");
      // we don't have a TXTRHNDL for the render target
      // bad things are going to happen if we allow this
      // so fail the context creation here!
      CONTEXT_FREE(ppdev, pccd->dwhContext);
      pccd->dwhContext = 0;
      pccd->ddrval = D3DHAL_CONTEXT_BAD;
      D3D_EXIT(DDHAL_DRIVER_HANDLED);
    }
#ifdef WINNT
//    ASSERTDD((DWORD) pccd->lpDDSLcl == newContext->pHndlList->ppTxtrHndlList[newContext->DDSHndl]->surfLcl, " render targ surfLcl's don't match!");
#endif
    if (pccd->lpDDSZLcl)
    {
      newContext->DDSZHndl =        pccd->lpDDSZLcl->lpSurfMore->dwSurfaceHandle;
      if ((NULL == newContext->pHndlList->ppTxtrHndlList) ||
          (NULL == newContext->pHndlList->ppTxtrHndlList[newContext->DDSZHndl]))
      {
        D3DPRINT(0, "ddiContextCreate - txtrHndl is NULL for the zbuffer");
        D3DPRINT(0, "                   failing context creation");
        // we don't have a TXTRHNDL for the z buffer
        // bad things are going to happen if we allow this
        // so fail the context creation here!
        CONTEXT_FREE(ppdev, pccd->dwhContext);
        pccd->dwhContext = 0;
        pccd->ddrval = D3DHAL_CONTEXT_BAD;
        D3D_EXIT(DDHAL_DRIVER_HANDLED);
      }
#ifdef WINNT
//      ASSERTDD((DWORD) pccd->lpDDSZLcl == newContext->pHndlList->ppTxtrHndlList[newContext->DDSZHndl]->surfLcl, " z buffer surfLcl's don't match!");
#endif
    }
    else
      newContext->DDSZHndl =        0;

    // go fill in contextId's of any TXTRHNDL's already created
    // for this context
    if (NULL != pHndlList->ppTxtrHndlList)
    {
      for (i = 1; i < (DWORD)pHndlList->ppTxtrHndlList[0]; i++)
      {
        if (0 != pHndlList->ppTxtrHndlList[i])
        {
#ifdef WINNT
          D3DPRINT(D3DDBGLVL, "  setting TxtrHndl[%ld] contextId to %8lXh", i, newContext);
#endif
          pHndlList->ppTxtrHndlList[i]->contextId = (DWORD)newContext;
        }
      }
    }

    // init state block info
    newContext->bSBRecMode =        FALSE;
    newContext->pCurrSB =           NULL;
    newContext->ppSBTable =         NULL;
  }
#endif

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
    // Initialize DX 8 Context Values
    newContext->lpIndices =         NULL;
    newContext->dwIndicesStride =   0;    
    newContext->dwVerticesStride =  0;
    newContext->dwNumVertices =     0;
    newContext->dwVBSizeInBytes =   0;
    newContext->dwVBHandle =        0;
    newContext->dwIndexHandle =     0;
    newContext->dwShaderHandle =    0;
#endif
  txtrFlavour(0xFFFFFFFA);
#if defined(TNL_PROFILE) && defined(TnL_HAL)
  TriClipStats(0xFFFFFFFA);
  MatrixStats(0xFFFFFFFA);
  InitFVFStats(0L);
#endif // TNL_PROFILE

  // Be sure to clear the chromaRange register because Glide will
  // set this register.  Fixes PRS #4335 (SpeedBusters). cws
  {
    CMDFIFO_PROLOG(cmdFifo);
    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chromaRange, 0xF ) );
    SETPD( cmdFifo, ghw->chromaRange, 0x0 );
    CMDFIFO_EPILOG( cmdFifo );
  }

#if (DX >= 7)
#ifdef TnL_HAL
  // Initialize the T&L related data in the RC  
  TLHAL_InitializeNewRC(newContext);
#endif
#endif

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
    // On DX8 D3DRENDERSTATE_TEXTUREPERSPECTIVE has been retired and is assumed 
    // to be set always to TRUE. We must make sure we are setting the hw up
    // correctly, so in order to do that we make an explicit setup call here 
    _renderFuncs[D3DRENDERSTATE_TEXTUREPERSPECTIVE](newContext, 1);
#endif

  pccd->ddrval = DD_OK;
  INS_EXIT( );
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}

/*-------------------------------------------------------------------
Function Name:  ddiContextDestroy
Description:    Entry point for ContextDestroy function.
Information:    DWORD __stdcall ddiContextDestroy(LPD3DHAL_CONTEXTDESTROYDATA pcdd)
Return:         DWORD DDHAL_DRIVER_HANDLED
                pccd->ddrval =
                                D3DHAL_CONTEXT_BAD
                             or DD_OK
-------------------------------------------------------------------*/
DWORD __stdcall ddiContextDestroy(LPD3DHAL_CONTEXTDESTROYDATA pcdd)
{
  SETUP_PPDEV(pcdd->dwhContext)
  D3D_ENTRY( "ddiContextDestroy" );
  INS_ENTRY( INSC_D3DCONTEXTDESTROY );

  //-------------------------------------------------------------------------
  // Delete the context. This callback is invoked when a context is to be destroyed.
  //-------------------------------------------------------------------------
  D3DPRINT( 255,"ddiContextDestroy, pcdd->dwhContext =%08lx", pcdd->dwhContext );

#ifdef WINNT
  D3DPRINT(D3DDBGLVL, "context=%lXh, ppdev=%8lXh", pcdd->dwhContext, ppdev);
  if (pcdd->dwhContext == _D3(lastContext))
  {
    _D3(last).colBufferAddr = 0;
    _D3(last).auxBufferAddr = 0;
    _D3(lastContext) = 0;
  }
#endif

#if defined(TNL_PROFILE) && defined(TnL_HAL)
  TriClipStats(0xFFFFFFFB);
  MatrixStats(0xFFFFFFFB);
  PrintFVFStats(0L);
#endif // TNL_PROFILE
  
#if (DX >= 7)
#ifdef TnL_HAL
  // Delete the T&L related data in the RC  
  TLHAL_DestroyRC((RC *)pcdd->dwhContext);
#endif
#endif

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  // release state blocks for this context
  ReleaseContextStateBlocks((RC *)pcdd->dwhContext);
#endif

  CONTEXT_FREE(ppdev, pcdd->dwhContext);
  if (pcdd->dwhContext == CONTEXT_INVALID)
  {
    D3DPRINT( 255,"ddiContextDestroy, bad context =0x08lx", pcdd->dwhContext );
    pcdd->ddrval = D3DHAL_CONTEXT_BAD;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  // for DX7, we'll free the TXTRHNDL's when the surface is destroyed
#else
  // When a process goes away not only do we clean up the context but
  // we have to free all the textures associated with this process.
  TXTRFREEHANDLESFORCONTEXT(ppdev, pcdd->dwhContext );
#endif

  txtrFlavour(0xFFFFFFFB);
  triangleFlavour(0, 0xFFFFFFFF, 0, 0 );
  #if defined( FLAVOR_PROFILE )
    renderTypes = 0;
  #endif

#if defined(LEGACY_APPS) && !defined(NT)
  _D3( legacyApp ) = 0;
#endif  // LEGACY_APPS

#if ENABLE_LOG_FILE
  {
    extern LONG g_retroCtxLive;
    g_retroCtxLive--;
    retroLogForce(ppdev, "retro3dfx CTX-DESTROY: h=%08lXh live=%ld 3dCnt=%ld\r\n",
                  pcdd->dwhContext, g_retroCtxLive, _FF(dd3DSurfaceCount));
  }
#endif

  pcdd->ddrval = DD_OK;
  INS_EXIT( );
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}

#ifndef WINNT
/*-------------------------------------------------------------------
Function Name:  ddiContextDestroyAll
Description:    Entry point for ContextDestroyAll function.
Information:    DWORD __stdcall ddiContextDestroyAll(LPD3DHAL_CONTEXTDESTROYALLDATA pcdd)
Return:         DWORD DDHAL_DRIVER_HANDLED
                pccd->ddrval = 	DD_OK
-------------------------------------------------------------------*/
DWORD __stdcall ddiContextDestroyAll(LPD3DHAL_CONTEXTDESTROYALLDATA pcdd)
{
  NT9XDEVICEDATA * ppdev;
  int i;

  D3D_ENTRY( "ddiContextDestroyAll" );
  INS_ENTRY( INSC_D3DCONTEXTDESTROYALL );

  //----------------------------------------------------------------
  // This callback is invoked when a process dies.  All the contexts
  // which were created by this context need to be destroyed.
  //----------------------------------------------------------------
  D3DPRINT( 255, "ddiContextDestroyAll, pcdd->dwPID =%08lx", pcdd->dwPID );

  for (i=0; i<NUM_DEVICES; i++)
  {

      ppdev = pDevices[i];
      if (NULL == ppdev)
         continue;

      CONTEXT_FREE_PID(ppdev, pcdd->dwPID );

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  // for DX7, we'll free the TXTRHNDL's when the surface is destroyed
#else
      // When a process goes away not only do we clean up the context but
      // we have to free all the textures that this process allocated.
      TXTRFREEHANDLESFORPROCESS(ppdev, pcdd->dwPID);
#endif
  }

#if defined(LEGACY_APPS) && !defined(NT)
  _D3( legacyApp ) = 0;
#endif  // LEGACY_APPS

  pcdd->ddrval = DD_OK;
  INS_EXIT( );
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}
#endif // ndef WINNT


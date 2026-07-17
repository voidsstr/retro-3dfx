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
** File name: tnlinit.c
**
** Description: Initialization code for software transformation and lighting
**
** $Revision: 14$
** $Date: 10/11/00 8:45:18 PM$
**
** $Log: 
**  14   3dfx      1.12.2.0    10/11/00 Brent           Forced check in to enforce
**       branching.
**  13   Napalm Shared1.12        03/29/00 Bob Johnston    Created User Memory
**       Split T&L Path and added index pre calculations.
**  12   Napalm Shared1.11        03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  11   Napalm Shared1.10        03/14/00 Bob Johnston    Changed User Mem
**       vertices to only use a min amount of memory in the SOAFVF_UM buff. Called
**       the setDX6State() from the FP branch in procprim.  Fixed
**       CanCreateExecBuff32 problem.  Decided to force VB creation to punt until I
**       fix all VB problems.
**  10   Napalm Shared1.9         03/01/00 Scott Kephart   SW T&L: Partial fix for
**       Napalm guardband clipping
**  9    Napalm Shared1.8         02/22/00 Bob Johnston    Fastpath rendering with
**       non-clipped vertices.  Texture coordinate handleing bug fixes, lots of bug
**       fixes.  Still showing some bugs in the fastpath.
**  8    Napalm Shared1.7         02/15/00 Scott Kephart   Updates to align TLLIGHT
**       structs on 32 byte boundaries
**  7    Napalm Shared1.6         02/04/00 Bob Johnston    Changes to support User
**       Memory vertex buffers for the fastpath.  Also cleaned up the vertex
**       processing loops with neater macros for better readablity.
**  6    Napalm Shared1.5         02/01/00 Scott Kephart   More lighting changes.
**       Better SSE matrix multiply code.
**  5    Napalm Shared1.4         01/28/00 Scott Kephart   Big T&L Merge: Init new
**       matrices
**  4    Napalm Shared1.3         01/10/00 Scott Kephart   Updates to lighting.
**       Split point and spot light functions. Only allow 8 active lights to reduce
**       branch mispredictions. Cleanup in lighting.
**  3    Napalm Shared1.2         12/13/99 Scott Kephart   Big T&L Update:
**       1. Improved T&L profiling code
**       2. Optimizations to scalar transformation and lighting code
**       3. Changes to the vertex buffer code to allow functionality under Windows
**       2000.
**  2    Napalm Shared1.1         11/03/99 Scott Kephart   add void to declaration
**       of TLHAL_DestroyRC
**  1    Napalm Shared1.0         10/25/99 Scott Kephart   
** $
 * 
 * 8     1/27/00 12:30p Skephart
 * 
 * 7     1/27/00 2:45a Skephart
 * 
 * 6     1/24/00 4:13p Skephart
 * Beginnings of the fast path
 * 
 * 5     1/20/00 2:26p Skephart
 * 
 * 4     1/19/00 9:50p Skephart
 * Add and Init SOA lighting function stubs. Allocate TL_TMP structure
 * 
 * 3     1/19/00 2:24p Skephart
 * Bob's FVF_COMP changes part II
** 
** 10    10/26/99 12:24a Skephart
** Merge in GB Clip changes from BobJ
** 
** 9     10/17/99 5:16p Skephart
** 
** 8     10/14/99 11:17a Skephart
** Bob's changes
** 
** 6     10/06/99 10:27p Skephart
** Cleanup
** 
** 5     10/06/99 3:14p Skephart
** Cleanup
** 
** 4     9/27/99 8:32p Skephart
** Move render context variables for TL HAL out of d3global.h, 
** and into tlglobal.h. All TL related variables are gathered under 
** pRc->tl.<variable>
** 
** 3     9/16/99 11:00a Skephart
** Finished up Init and Destroy Code
** 
** 2     9/15/99 11:21p Skephart
** Added more init code
** 
** 1     9/15/99 11:16a Skephart
** Initialization code for Transformation and lighting
*/

#include "precomp.h"

#ifndef WINNT
#include <windows.h>
#include <stddef.h>
#include <d3dhal.h>
#include "d3global.h"
#include "ddrawi.h"
#include "ddglobal.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "d3contxt.h"
#define  FX_DEFINE_MACROS
#include "ddfxnt95.h"
#endif


#ifdef WINNT
#define __NTDDKCOMP__
#include "dmemmgr.h"
#endif

#if( DX >= 7 )
#ifdef TnL_HAL

/*
 ** TLHAL_InitializeNewRC
 *
 *  FILENAME: C:\project\NAPALM\d3d\tnlinit.c
 *
 *  PARAMETERS: 
 *    RC *pRc -- pointer to the rendering context to be initialized
 *
 *  DESCRIPTION:
 *    Initializes the T&L state variables within the RC
 *
 *  RETURNS:
 *
 */

void TLHAL_InitializeNewRC(RC *pRc)
{
  int i;
  NT9XDEVICEDATA *ppdev = pRc->ppdev;

  // Clear all this junk Out
  memset((void *) &pRc->tl.InFVF, 0, sizeof(FVF_COMP));
  memset((void *) &pRc->tl.SOAFVF, 0, sizeof(FVF_COMP));
  memset((void *) &pRc->tl.TLFVF, 0, sizeof(FVF_COMP));
  memset((void *) &pRc->tl.TLBN, 0, sizeof(FVF_COMP));

  pRc->tl.dwTextureCoordSizeTotal = 0;
  for (i = 0; i <  D3DDP_MAXTEXCOORD; i++)
  {
    pRc->tl.dwTexCoordSize[i] = 0;
  }

  memset((void *) &pRc->tl.TLVBuf, 0, sizeof(AlignedBuffer32));

  pRc->tl.numVertexBlends = 0;
  pRc->tl.dwDirtyFlags = 0;
  pRc->tl.dwTLState = 0;

  pRc->tl.clipPlaneEnable = 0;
  pRc->tl.clipUnion = 0;        
  pRc->tl.clipIntersection = 0; 

  memset((void *) &pRc->tl.clipping, 0, sizeof(TLCLIPPING));
  memset((void *) &pRc->tl.UserClipPlanes, 0, sizeof(TLVECTOR4)*TLMAX_USER_CLIPPLANES);
  memset((void *) &pRc->tl.xfmUserClipPlanes, 0, sizeof(TLUSERCLIPPLANE)*TLMAX_USER_CLIPPLANES);

  memset((void *) &pRc->tl.Viewport, 0, sizeof(D3DVIEWPORT7));
  memset((void *) &pRc->tl.ViewData, 0, sizeof(TLVIEWPORTDATA));
  memset((void *) &pRc->tl.TransformData, 0, sizeof(TLTRANSFORMDATA));
  memset((void *) &pRc->tl.xfmProj, 0, sizeof(TLMATRIX));
  memset((void *) &pRc->tl.xfmView, 0, sizeof(TLMATRIX));
  memset((void *) &pRc->tl.xfmWorld, 0, sizeof(TLMATRIX)*TLMAX_WORLD_MATRICES);


  //*****************************************************
  // Create Aligned. fixed size memory buffers for the 
  // following resources needed in the T&L HAL
  //*****************************************************

  // Create the Main transformed and lit output Vertex Buffer
  // for both the Slowpath and Fastpath to use.
  AB32_Create(pRc->ppdev, &pRc->tl.TLVBuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.TLVBuf, (D3DMAXNUMVERTICES * MAX_TLBUFF_VERT_SIZE));
  
  // Assing the Slowpath pointer to the allocation
  pRc->tl.TLFVF.lpvData = (LPVOID) pRc->tl.TLVBuf.alignedBuf;

  // The Fastpath shares the TL_BUFF with the slowpath
  // Assign the address of the TLBN vertex pointer to the
  // beginning of the TL_BUFF
  pRc->tl.TLBN.lpvData = (LPVOID) pRc->tl.TLVBuf.alignedBuf;

  // Create the slowpath's clip code buffer
  AB32_Create(pRc->ppdev, &pRc->tl.ClipFlagBuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.ClipFlagBuf, (D3DMAXNUMVERTICES * sizeof(DWORD)));
  // Assign this buffer address to the TL context data struct
  pRc->tl.pClipBuf = (TLCLIPCODE *) pRc->tl.ClipFlagBuf.alignedBuf;


  // Create the Fastpath's USer Memory SOA buffer
  // Allocate a triple buffer SOAGroup
  AB32_Create(pRc->ppdev, &pRc->tl.SOAUMBuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.SOAUMBuf, ( 12 * MAX_TLBUFF_VERT_SIZE));

  AB32_Create(pRc->ppdev, &pRc->tl.clipping.ClipBuf);
  //BobJ 10/13/1999  Allocate a fixed Output ClipBuf of 4K bytes
  AB32_Grow(pRc->ppdev, &pRc->tl.clipping.ClipBuf, 0x2000);

  AB32_Create(pRc->ppdev, &pRc->tl.xfmCurrentBuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmCurrentBuf, sizeof(TLMATRIX)*TLMAX_WORLD_MATRICES);

  AB32_Create(pRc->ppdev, &pRc->tl.xfmToEyeBuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmToEyeBuf, sizeof(TLMATRIX)*TLMAX_WORLD_MATRICES);

  AB32_Create(pRc->ppdev, &pRc->tl.xfmToEyeInvBuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmToEyeInvBuf, sizeof(TLMATRIX)*TLMAX_WORLD_MATRICES);

  AB32_Create(pRc->ppdev, &pRc->tl.xfmToEyeInvTBuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmToEyeInvTBuf, sizeof(TLMATRIX));

  AB32_Create(pRc->ppdev, &pRc->tl.TL_TMP_Buff);
  AB32_Grow(pRc->ppdev, &pRc->tl.TL_TMP_Buff, SIZE_TL_TMP);
  pRc->tl.pTL = pRc->tl.TL_TMP_Buff.alignedBuf;
  
  AB32_Create(pRc->ppdev, &pRc->tl.xfmCurrentSOABuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmCurrentSOABuf, sizeof(SOA_MATRIX));
  pRc->tl.lpxfmCurrentSOA = (SOA_MATRIX *) pRc->tl.xfmCurrentSOABuf.alignedBuf;

  AB32_Create(pRc->ppdev, &pRc->tl.xfmToEyeSOABuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmToEyeSOABuf, sizeof(SOA_MATRIX));
  pRc->tl.lpxfmToEyeSOA = (SOA_MATRIX *) pRc->tl.xfmToEyeSOABuf.alignedBuf;

  AB32_Create(pRc->ppdev, &pRc->tl.xfmToEyeInvTSOABuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmToEyeInvTSOABuf, sizeof(SOA_MATRIX));
  pRc->tl.lpxfmToEyeInvTSOA = (SOA_MATRIX *) pRc->tl.xfmToEyeInvTSOABuf.alignedBuf;

  for(i = 0; i < TLMAX_WORLD_MATRICES; i++)
  {
     pRc->tl.lpxfmCurrent[i] = (TLMATRIX *)((LPBYTE)pRc->tl.xfmCurrentBuf.alignedBuf + (i*sizeof(TLMATRIX))); 
     pRc->tl.lpxfmToEye[i] = (TLMATRIX *)((LPBYTE)pRc->tl.xfmToEyeBuf.alignedBuf + (i*sizeof(TLMATRIX))); 
     pRc->tl.lpxfmToEyeInv[i] = (TLMATRIX *)((LPBYTE)pRc->tl.xfmToEyeInvBuf.alignedBuf + (i*sizeof(TLMATRIX))); 
  }

  pRc->tl.lpxfmToEyeInvT = (TLMATRIX *)pRc->tl.xfmToEyeInvTBuf.alignedBuf;


  pRc->tl.primType = 0;
  pRc->tl.dwNumVertices = 0;
  pRc->tl.dwNumIndices = 0;
  pRc->tl.pIndices = 0;
  pRc->tl.pLightArray = 0;
  pRc->tl.pLightAlloc = 0;
  pRc->tl.dwLightArraySize = 0;
  
  memset((void *) &pRc->tl.LightVertexTable, 0, sizeof(TLLIGHTVERTEX_FUNC_TABLE));
  memset((void *) &pRc->tl.lighting, 0, sizeof(TLLIGHTING));
  memset((void *) &pRc->tl.Material, 0, sizeof(D3DMATERIAL7));

  pRc->tl.AmbMaterialSrc = 0;
  pRc->tl.DfusMaterialSrc = 0;
  pRc->tl.SpecMaterialSrc = 0;
  pRc->tl.EmisMaterialSrc = 0;

  pRc->tl.LightVertexTable.pfnDirectional = TLLV_Directional;
  pRc->tl.LightVertexTable.pfnSpot = TLLV_Spot;
  pRc->tl.LightVertexTable.pfnPoint = TLLV_Point;
#ifdef SSECPP
  pRc->tl.LightVertexTable.pfnDirectionalSOA = TLLV_DirectionalSOA;
  pRc->tl.LightVertexTable.pfnSpotSOA = TLLV_SpotSOA;
  pRc->tl.LightVertexTable.pfnPointSOA = TLLV_PointSOA;
#else
  pRc->tl.LightVertexTable.pfnDirectionalSOA = 0;
  pRc->tl.LightVertexTable.pfnSpotSOA = 0;
  pRc->tl.LightVertexTable.pfnPointSOA = 0;
#endif
  
  //
  // Guardband parameters
  //

  if (IS_NAPALM && _D3(GuardbandClipping)) {
    pRc->tl.dwTLState |= TLPV_GUARDBAND; 
    // BobJ -- We now initialize the guarband clipping T&L clip bit in SetDX6State
    pRc->tl.ViewData.minXgb = (H5_GB_LEFT);
    pRc->tl.ViewData.maxXgb = H5_GB_RIGHT;
    pRc->tl.ViewData.minYgb = (H5_GB_TOP);
    pRc->tl.ViewData.maxYgb = H5_GB_BOTTOM;
  }
  
  // Initialize the "bad" flags -- things that disallow the fast path TL code
  pRc->tl.dwTLBadFlags = (TLPV_COLORVERTEXAMB | TLPV_COLORVERTEXDIFF | TLPV_COLORVERTEXSPEC | TLPV_VERTEXBLENDNEEDED);

}  

/*
 ** TLHAL_DestroyRC
 *
 *  FILENAME: C:\project\NAPALM\d3d\tnlinit.c
 *
 *  PARAMETERS:
 *    RC *pRc -- rendering context to be destroyed
 *
 *  DESCRIPTION:
 *    Destroy all T&L state related variables
 *
 *  RETURNS:
 *
 */

void TLHAL_DestroyRC(RC *pRc)
{
  NT9XDEVICEDATA *ppdev = pRc->ppdev;

  // Delete memory associated with temporary vertex buffer and clip flag buffer
  AB32_Destroy(pRc->ppdev, &pRc->tl.TLVBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.ClipFlagBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.SOAUMBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.clipping.ClipBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmCurrentBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmToEyeBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmToEyeInvBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmToEyeInvTBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmCurrentSOABuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmToEyeSOABuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmToEyeInvTSOABuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.TL_TMP_Buff);
  DXFREE(pRc->tl.pLightAlloc);
  pRc->tl.pLightArray = 0;
  pRc->tl.pLightAlloc = 0;
  pRc->tl.dwLightArraySize = 0;  

}  

#endif    // #if( DX >= 7 )
#endif    // #ifdef TnL_HAL








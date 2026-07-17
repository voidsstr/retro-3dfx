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
** $Revision: 31$
** $Date: 10/11/00 8:49:40 PM$
**
** $Log: 
**  31   3dfx      1.12.1.4.1.1210/11/00 Brent           Forced check in to enforce
**       branching.
**  30   3dfx      1.12.1.4.1.1110/10/00 Allen Hansen    added vertex blends to the
**       optimized paths
**  29   3dfx      1.12.1.4.1.1009/22/00 Allen Hansen    implemented texgen and
**       texture transformation to the fastpaths
**  28   3dfx      1.12.1.4.1.908/31/00 Allen Hansen    added SSE2 support
**  27   3dfx      1.12.1.4.1.808/29/00 Allen Hansen    enabled colorvertex support
**       for sse path
**  26   3dfx      1.12.1.4.1.708/27/00 Allen Hansen    added colorvertex support
**       for 3dnow path
**  25   3dfx      1.12.1.4.1.608/21/00 Allen Hansen    added
**       TLPV_NOT_SOLID_FILL_MODE to dwTLBadFlags
**  24   3dfx      1.12.1.4.1.507/29/00 Allen Hansen    Added pSynthIndices for
**       converting non-indexed tri primitives to indexed trs's
**  23   3dfx      1.12.1.4.1.407/24/00 Allen Hansen    Rewrote cpu detection code,
**       cpuType is now defined in cpu.h, all new code should be protected by
**       #ifdef WINNT
**  22   3dfx      1.12.1.4.1.307/08/00 Allen Hansen    fixed sse path detection
**       bug
** 
**  21   3dfx      1.12.1.4.1.207/05/00 Allen Hansen    Changed "TextureLighting"
**       to "TransformAndLighting"
**  20   3dfx      1.12.1.4.1.107/03/00 Allen Hansen    added support for 3DNow
**       special cased asm lighting functions
**  19   3dfx      1.12.1.4.1.006/25/00 Allen Hansen    fixed FTOI function (was
**       saving integer to qword instead of dword), wrote SwizzleScaler2SOAFLOAT()
**  18   3dfx      1.12.1.4    06/02/00 Allen Hansen    Set up the mxcsr when a new
**       tl_rendering_context is created
**  17   3dfx      1.12.1.3    06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  16   3dfx      1.12.1.2    05/11/00 Scott Kephart   More optimizations for
**       lighting
**  15   3dfx      1.12.1.1    05/09/00 Bob Johnston    Tweak to Dynamic GB.  Don't
**       immediately set the dirty flag until after the 1st 16 frames.
**  14   3dfx      1.12.1.0    05/05/00 Bob Johnston    Implimented Dynamic
**       Guardband clipper for SW T&L HAL
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
 * Add and Init SOA lighting function stubs. Allocate TL_SOATMP structure
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

#include "regkeys.h"

#ifdef K6_2
#include "k6_2.h"
#include "d6global.h"
#endif // K6_2

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
 *  RETURNS: D3D_OK or !D3D_OK
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

  // Code Path's for various cpu's
  if(_D3(TransformAndLighting) != TnL_DISABLED)
  {
    pRc->tl.dwTLCpuCodePath = TL_CODEPATH_DEFAULT;

    if(_D3(TransformAndLighting) == TnL_OPTIMISED) 
    {
#ifdef WINNT	// OLD CPUID CODE
	  if(CPUTYPE & P6_INTELCPU_WITH_KNI)
#else
	  if(CPUTYPE & CPU_FEATURE_SSE)
#endif
      {
        pRc->tl.dwTLCpuCodePath = TL_CODEPATH_SSE;

  	    // Let's set the mxcsr now (probably the wrong place to do this)
	    _asm stmxcsr dword ptr [i]
  	    i &= 0xffff1fff;	// mask off rounding and flush-to-zero bits
	    i |= (0x3f <<  7);	// make sure the exceptions are all masked
	    i |= (0x00 << 13);	// set rounding to nearest
	    i |= (0x01 << 15); 	// set the flush-to-zero
	    _asm ldmxcsr dword ptr [i]

	    if(CPUTYPE & CPU_FEATURE_SSE2)	// Pentium 4
          pRc->tl.dwTLCpuCodePath |= TL_CODEPATH_SSE2;
	  }
#ifdef WINNT	// OLD CPUID CODE
      else if(detect3DX())
#else
	  else if(CPUTYPE & CPU_FEATURE_3DNOW)
#endif
	  {
        if((CPUTYPE & CPU_FEATURE_AMMX) && (CPUTYPE & CPU_FEATURE_3DNOWX))
          pRc->tl.dwTLCpuCodePath = TL_CODEPATH_3DNOW | TL_CODEPATH_3DNOWEXT;
		else
          pRc->tl.dwTLCpuCodePath = TL_CODEPATH_3DNOW;
	  }
	}
  }

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

  // Create the indexed primitive buffer.  We'll use it to convert primitives that 
  // aren't non-indexed triangle lists into indexed triangle lists
  AB32_Create(pRc->ppdev, &pRc->tl.pSynthIndices);
  AB32_Grow(pRc->ppdev, &pRc->tl.pSynthIndices, (sizeof(unsigned short) * (TLMAXNUMVERTICES+3)));

  //BobJ 10/13/1999  Allocate a fixed Output ClipBuf of 4K bytes
  AB32_Create(pRc->ppdev, &pRc->tl.clipping.ClipBuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.clipping.ClipBuf, 0x2000);

  AB32_Create(pRc->ppdev, &pRc->tl.xfmCurrentBuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmCurrentBuf, sizeof(TLMATRIX) * TLMAX_WORLD_MATRICES);

  AB32_Create(pRc->ppdev, &pRc->tl.xfmToEyeBuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmToEyeBuf, sizeof(TLMATRIX) * TLMAX_WORLD_MATRICES);

  AB32_Create(pRc->ppdev, &pRc->tl.xfmToEyeInvBuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmToEyeInvBuf, sizeof(TLMATRIX) * TLMAX_WORLD_MATRICES);

  AB32_Create(pRc->ppdev, &pRc->tl.xfmToEyeInvTBuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmToEyeInvTBuf, sizeof(TLMATRIX) * TLMAX_WORLD_MATRICES);

  AB32_Create(pRc->ppdev, &pRc->tl.xfmCurrentSOABuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmCurrentSOABuf, sizeof(SOA_MATRIX) * TLMAX_WORLD_MATRICES);

  AB32_Create(pRc->ppdev, &pRc->tl.xfmToEyeSOABuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmToEyeSOABuf, sizeof(SOA_MATRIX) * TLMAX_WORLD_MATRICES);

  AB32_Create(pRc->ppdev, &pRc->tl.xfmToEyeInvTSOABuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmToEyeInvTSOABuf, sizeof(SOA_MATRIX) * TLMAX_WORLD_MATRICES);

  for(i = 0; i < TLMAX_WORLD_MATRICES; i++)
  {
     pRc->tl.lpxfmCurrent[i]		= (TLMATRIX*)((LPBYTE)pRc->tl.xfmCurrentBuf.alignedBuf + (i*sizeof(TLMATRIX))); 
     pRc->tl.lpxfmToEye[i]			= (TLMATRIX*)((LPBYTE)pRc->tl.xfmToEyeBuf.alignedBuf + (i*sizeof(TLMATRIX))); 
     pRc->tl.lpxfmToEyeInv[i]		= (TLMATRIX*)((LPBYTE)pRc->tl.xfmToEyeInvBuf.alignedBuf + (i*sizeof(TLMATRIX))); 
     pRc->tl.lpxfmToEyeInvT[i]		= (TLMATRIX*)((LPBYTE)pRc->tl.xfmToEyeInvTBuf.alignedBuf + (i*sizeof(TLMATRIX)));
     pRc->tl.lpSOAxfmCurrent[i]		= (SOA_MATRIX*)((LPBYTE)pRc->tl.xfmCurrentSOABuf.alignedBuf + (i*sizeof(SOA_MATRIX)));
     pRc->tl.lpSOAxfmToEye[i]		= (SOA_MATRIX*)((LPBYTE)pRc->tl.xfmToEyeSOABuf.alignedBuf + (i*sizeof(SOA_MATRIX)));
	 pRc->tl.lpSOAxfmToEyeInvT[i]	= (SOA_MATRIX*)((LPBYTE)pRc->tl.xfmToEyeInvTSOABuf.alignedBuf + (i*sizeof(SOA_MATRIX)));
  }

  AB32_Create(pRc->ppdev, &pRc->tl.TL_SOATMP_Buff);
  AB32_Grow(pRc->ppdev, &pRc->tl.TL_SOATMP_Buff, SIZE_TL_SOATMP);
  pRc->tl.pTL = pRc->tl.TL_SOATMP_Buff.alignedBuf;

  AB32_Create(pRc->ppdev, &pRc->tl.TL_K3DTMP_Buff);
  AB32_Grow(pRc->ppdev, &pRc->tl.TL_K3DTMP_Buff, sizeof(TL_K3DTMP));
  pRc->tl.pTLK = pRc->tl.TL_K3DTMP_Buff.alignedBuf;


  AB32_Create(pRc->ppdev, &pRc->tl.xfmTxtrTBuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmTxtrTBuf, sizeof(TLMATRIX)*D3DDP_MAXTEXCOORD);

  AB32_Create(pRc->ppdev, &pRc->tl.xfmTxtrSOABuf);
  AB32_Grow(pRc->ppdev, &pRc->tl.xfmTxtrSOABuf, sizeof(SOA_MATRIX)*D3DDP_MAXTEXCOORD);

  for(i = 0; i < D3DDP_MAXTEXCOORD; i++)
  {
     pRc->tl.lpxfmTxtrT[i]   = (TLMATRIX *)((LPBYTE)pRc->tl.xfmTxtrTBuf.alignedBuf + (i*sizeof(TLMATRIX)));
     pRc->tl.lpxfmTxtrSOA[i] = (SOA_MATRIX *)((LPBYTE)pRc->tl.xfmTxtrSOABuf.alignedBuf + (i*sizeof(SOA_MATRIX)));
  }


  pRc->tl.primType = 0;
  pRc->tl.dwNumVertices = 0;
  pRc->tl.dwNumIndices = 0;
  pRc->tl.pIndices = 0;
  pRc->tl.lighting.pLightArray = 0;
  pRc->tl.lighting.pLightAlloc = 0;
  pRc->tl.lighting.dwLightArraySize = 0;
  pRc->tl.transformedVB = FALSE;
  pRc->tl.dwDirtyXfmTxtr = 0;
  
  memset((void *) &pRc->tl.lighting.LightVertexTable, 0, sizeof(TLLIGHTVERTEX_FUNC_TABLE));
  memset((void *) &pRc->tl.lighting, 0, sizeof(TLLIGHTING));
  memset((void *) &pRc->tl.lighting.Material, 0, sizeof(D3DMATERIAL7));

  pRc->tl.lighting.AmbMaterialSrc = 0;
  pRc->tl.lighting.DfusMaterialSrc = 0;
  pRc->tl.lighting.SpecMaterialSrc = 0;
  pRc->tl.lighting.EmisMaterialSrc = 0;

  pRc->tl.lighting.LightVertexTable.pfnDirectional = TLLV_Directional;
  pRc->tl.lighting.LightVertexTable.pfnSpot = TLLV_Spot;
  pRc->tl.lighting.LightVertexTable.pfnPoint = TLLV_Point;

  pRc->tl.lighting.LightVertexTable.pfnDirectionalK3d = TLLV_DirectionalK3D_C;
  pRc->tl.lighting.LightVertexTable.pfnSpotK3d = TLLV_SpotK3D_C;
  pRc->tl.lighting.LightVertexTable.pfnPointK3d = TLLV_PointK3D_C;
  
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
  
  // Bob J --  Initialize Dynamic Guardband
  pRc->tl.dwFrameCounter = 0;
  pRc->tl.dwFramesQueued = 0;


  // Initialize the "bad" flags -- things that disallow the fast path TL code
  pRc->tl.dwTLBadFlags = 
		TLPV_NOT_SOLID_FILL_MODE |	// points, lines, or wireframe
		TLPV_NON_2D_TEXTURE;		// non-2d input texture format


  // One-time function selections
  if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE2)
    pRc->tl.lighting.ConvertPackedSOAColortoSOA_RGB = &ConvertPackedSOAColortoSOA_RGB_SSE2;
  else
    pRc->tl.lighting.ConvertPackedSOAColortoSOA_RGB = &ConvertPackedSOAColortoSOA_RGB_SSE;

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
  AB32_Destroy(pRc->ppdev, &pRc->tl.pSynthIndices);
  AB32_Destroy(pRc->ppdev, &pRc->tl.clipping.ClipBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmCurrentBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmToEyeBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmToEyeInvBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmToEyeInvTBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmCurrentSOABuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmToEyeSOABuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmToEyeInvTSOABuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.TL_SOATMP_Buff);
  AB32_Destroy(pRc->ppdev, &pRc->tl.TL_K3DTMP_Buff);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmTxtrTBuf);
  AB32_Destroy(pRc->ppdev, &pRc->tl.xfmTxtrSOABuf);
  DXFREE(pRc->tl.lighting.pLightAlloc);
  pRc->tl.lighting.pLightArray = 0;
  pRc->tl.lighting.pLightAlloc = 0;
  pRc->tl.lighting.dwLightArraySize = 0;  

}  

#endif    // #if( DX >= 7 )
#endif    // #ifdef TnL_HAL

/*
** Copyright (c) 2000, 3Dfx Interactive, Inc.
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
** File name: soaprim.c
**
** Description: Transformation and Lighting Primitives for DX7
**
** $Revision: 79$
** $Date: 10/18/00 5:40:55 PM$
**
** $Log: 
**  79   3dfx      1.49.1.6.1.2110/18/00 Allen Hansen    Changed SSE T&L tag scheme
**       from a bitfield to marking the vert T&L'd in one of the textures
**  78   3dfx      1.49.1.6.1.2010/11/00 Brent           Forced check in to enforce
**       branching.
**  77   3dfx      1.49.1.6.1.1910/10/00 Allen Hansen    added profiling code
**  76   3dfx      1.49.1.6.1.1809/28/00 Allen Hansen    started vertex blend
**       support in fastpath (still ifdef'd out)
**  75   3dfx      1.49.1.6.1.1709/23/00 Allen Hansen    more code cleanup (get rid
**       of reminents of TLBN_CLIP)
**  74   3dfx      1.49.1.6.1.1609/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  73   3dfx      1.49.1.6.1.1509/22/00 Allen Hansen    implemented texgen and
**       texture transformation to the fastpaths
**  72   3dfx      1.49.1.6.1.1409/15/00 Allen Hansen    In the 3DNow path, moved
**       the texture copy from before culling to after culling ... saves a copy
**       which may not need to be done at all.  Also will make the texgen code
**       simpler.
**  71   3dfx      1.49.1.6.1.1309/15/00 Allen Hansen    Got rid of the TLBN_CLIP
**       structure (with homogenous coords), all output verts, clipped or not, use
**       the TLBN structure.
**  70   3dfx      1.49.1.6.1.1209/05/00 Allen Hansen    Fixed bug in the swizzle
**       code: we were always swizzling blocks of 4 verts.  If the # of verts
**       wasn't divisible by 4, we always swizzled the last group of 4.  We can
**       page fault (or GPF) if the end of a VB is near the end of a page (or
**       segment). 
**  69   3dfx      1.49.1.6.1.1109/03/00 Allen Hansen    fixed the third 3DNow
**       outer loop so autostripping can happen, added comments, code cleanup
**  68   3dfx      1.49.1.6.1.1008/31/00 Allen Hansen    tweaked the thresholds in
**       the 3DNow code for when we have a large vertex buffer but only use a small
**       portion (used Evolva benchmark), some code cleanup
**  67   3dfx      1.49.1.6.1.908/29/00 Allen Hansen    removed dead code
**  66   3dfx      1.49.1.6.1.808/27/00 Allen Hansen    implemented 2 more outer
**       loops in the 3dnow code, one to handle when most of the verts are used by
**       a primitive, and one to handle when only use a few verts out of a large VB
**       (Messiah)
**  65   3dfx      1.49.1.6.1.708/21/00 Allen Hansen    changed from sse intrinsics
**       to inline assembly, check that verts are dirty before T&L
**  64   3dfx      1.49.1.6.1.607/29/00 Allen Hansen    Changed the check for
**       calling the clipped rendering code vs. the non-clipped (check clip codes
**       against dwClipMask instead of 0)
**  63   3dfx      1.49.1.6.1.507/24/00 Allen Hansen    whql bug fixes (T&L tests)
**  62   3dfx      1.49.1.6.1.407/08/00 Allen Hansen    3dnow code cleanup
**  61   3dfx      1.49.1.6.1.307/05/00 Allen Hansen    added asm clip code
**       generation to 3dnow code
**  60   3dfx      1.49.1.6.1.207/03/00 Allen Hansen   
**       FP_IndexedTriangleList2_K3D() function work
**  59   3dfx      1.49.1.6.1.106/29/00 Allen Hansen    Changed K3D outer loop,
**       wrote 3dnow cull check
**  58   3dfx      1.49.1.6.1.006/25/00 Allen Hansen    added a couple of missing
**       emms instructions, added 3dnow procprim function
**  57   3dfx      1.49.1.6    06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  56   3dfx      1.49.1.5    05/30/00 Allen Hansen    Added the
**       XformDeswiz4_DevCoord_SOA_GEOM functions to
**       FP_IndexedTriangleList2_SOA_Split, the transformation code calls one
**       function instead of several, gain's about 1/2 fps
**  55   3dfx      1.49.1.4    05/22/00 Allen Hansen    Implemented 2-Pass outer
**       loop for small primitives, it's marginally faster so the code is currently
**       disabled.
**  54   3dfx      1.49.1.3    05/16/00 Bob Johnston    Consolodating VERT_BUFF and
**       TnL_HAL defines to just use TnL_HAL
**  53   3dfx      1.49.1.2    05/11/00 Allen Hansen    Changed prefetch from
**       Prefetch0 to PrefetchNTA, changed pIndx from short* to unsigned short*
**       (because we'd blow up if index list was greater than 32K), put
**       Xform_DevCoord_SOA_GEOM_Funct* in the RC
**  52   3dfx      1.49.1.1    05/10/00 Allen Hansen    Optimized SetupTL_TMP
**       function
**  51   3dfx      1.49.1.0    05/03/00 Allen Hansen    Moved deswizzling of color,
**       fog, and textures into the texture transform functions, which eliminates
**       storing and loading these params
**  50   Napalm    1.49        04/26/00 Bob Johnston    use of local function
**       pointers to help eliminate branching in the vertex loops.  Setup a Fn
**       pointer for Xform_DevCoord_SOA_GEOM
**  49   Napalm    1.48        04/19/00 Bob Johnston    Cleanup of the code and
**       removal of test code references
**  48   Napalm    1.47        04/18/00 Allen Hansen    Fixed buy in
**       FP_IndexedTriangleList2_SOA_UM_Split(), was vCurIdxX wasn't being updated
**       in the lighting code so some SOA_Verts getting stomped on
**  47   Napalm    1.46        04/17/00 Allen Hansen    Worked on the new version
**       of FP_IndexedTriangleList2_SOA_Split(), knocked off a few more assembly
**       instructions so the function went from 18.2% to 17.5% in test 1.
**  46   Napalm    1.45        04/17/00 Bob Johnston    Optimized the VB Split
**       vertex loop to use a byte array.  Shows a small improvement perfrormance.
**  45   Napalm    1.44        04/12/00 Allen Hansen    1) changed input param to
**       TNL rendering function from *RC to *TL_RENDER_DATA
**       2) added prefetching to single-swizzle code, requires new file
**       asoaswiz.asm (which also changed the makefile)
**  44   Napalm    1.43        04/11/00 Bob Johnston    Optimization's in the
**       Device Coordinate Transform code for retrieving scales and offsets in SOA
**       form, thus minimizing the only fly shuffle and HW offset adds.
**  43   Napalm    1.42        04/10/00 Allen Hansen    Worked prefetching: Changed
**       FP_IndexedTriangleList2_SOA_Split() to transform 4 soa groups instead of
**       1.  This required modifing AllocateSOAFVF().  I added a version of
**       FP_Xform_4Vert_SOA() that prefetches the next SOA group.  This still needs
**       optimization, but almost all cache-read misses are hidden.
**  42   Napalm    1.41        04/04/00 Bob Johnston    Bug fixes for Split VB path
**       and Full UM
**  41   Napalm    1.40        04/04/00 Scott Kephart   SW T&L change - cleaned up
**       the profiling code
**  40   Napalm    1.39        04/03/00 Allen Hansen    SW T&L Only:
**       Optimizations to asm rendering code,
**       Combined version of clip/cull check optimized cull check,
**       asm version of deswizzle
** 
** 
**  39   Napalm    1.38        04/03/00 Scott Kephart   SW T&L Change - added
**       vertex order profiling code.
**  38   Napalm    1.37        03/29/00 Bob Johnston    Created User Memory Split
**       T&L Path and added index pre calculations.
**  37   Napalm    1.36        03/23/00 Bob Johnston    Scott and Bob's changes to
**       split up the tranformation and lighting in the vertex processing loop for
**       improved VB primitive perfromance.
**  36   Napalm    1.35        03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  35   Napalm    1.34        03/16/00 Bob Johnston    Got Vertex Buffers working
**       correctly
**  34   Napalm    1.33        03/14/00 Bob Johnston    Changed User Mem vertices
**       to only use a min amount of memory in the SOAFVF_UM buff. Called the
**       setDX6State() from the FP branch in procprim.  Fixed CanCreateExecBuff32
**       problem.  Decided to force VB creation to punt until I fix all VB
**       problems.
**  33   Napalm    1.32        03/08/00 Scott Kephart   Re-shuffle of T&L code.
**  32   Napalm    1.31        03/04/00 Bob Johnston    Increased max vertices for
**       VB and User Mem to 8K.  This fixes 3DMark2K's problems.  Also but a fail
**       over check to punt to the slow path  when the driver sees more than 8K of
**       vertices inthe T&L HAL.  This number can be easily adjusted
**  31   Napalm    1.30        03/03/00 Bob Johnston    Fix for 3DMark2K.  Changed
**       the way we size the SOFVF buff.  Turned off testcode texture coord back
**       out for now and forced Setup_Render before getting texture coord scale and
**       offset
**  30   Napalm    1.29        03/03/00 Scott Kephart   SW T&L: re-wrote
**       DeSwizzleSOAToTLBN This allows  us to change the ordering of members in
**       the TLBN struct without breaking code. 
**       Added 1 dword of padding to TLBN. Moved clip_code to before the texture
**       coordinates in TLBN_CLIP. (Same position as pad DWORD in TLBN).
**  29   Napalm    1.28        03/02/00 Scott Kephart   SW T&L: Fixes for Napalm
**       guardband clip code generation. Fixes for test code.
**  28   Napalm    1.27        03/01/00 Allen Hansen    Fixed vertex fog for T&L
**  27   Napalm    1.26        02/28/00 Bob Johnston    Optimization to carry XMM
**       register values between blocks of code.  Turned on bt the
**       CARRY_XMM_REG_VALUES define in tlglobal.h
**  26   Napalm    1.25        02/25/00 Bob Johnston    Optimized Texture Coord
**       handleing in DevCoordXform.  Made test code acurate for texture coords
**       from TL_TMP
**  25   Napalm    1.24        02/25/00 Scott Kephart   Added emms to the end of
**       Xform_Light_4Vertex_SOA
**  24   Napalm    1.23        02/24/00 Matt McClure    Modifications to return
**       texture coordinate copy to its original state.  Re-Enabled Bob's trivial
**       acception.
**  23   Napalm    1.22        02/23/00 Matt McClure    Modifications to grab the
**       original texture coordinates for the clipping path.  Also, disabled the
**       trivial acception in the clipping path.  It is currently impossible the
**       way the cache and TLBN_CLIP is structured, and because the clipping code
**       doesn't know about the indices.
**  22   Napalm    1.21        02/23/00 Bob Johnston    Fastpath clipping with auto
**       stripping, trivial accept, reject and better configuration.
**  21   Napalm    1.20        02/22/00 Bob Johnston    Fastpath rendering with
**       non-clipped vertices.  Texture coordinate handleing bug fixes, lots of bug
**       fixes.  Still showing some bugs in the fastpath.
**  20   Napalm    1.19        02/21/00 Bob Johnston    Optimized Viewport
**       transform's use of texture coords and made changes in the swizzle of Text
**       coords into SOAFVF.  Also added support in the vertex proceesing loop to
**       call on the newly added fast path rendering code.  Added necessary changes
**       to the T&L context data structure to support rendering
**  19   Napalm    1.18        02/16/00 Scott Kephart   Fixed copy of specular and
**       diffuse for non-lit vertices
**  18   Napalm    1.17        02/16/00 Bob Johnston    Completed full viewport
**       transformation code to spec.  Updated test code to use new viewport xform.
**  17   Napalm    1.16        02/15/00 Scott Kephart   Fix for Xform_DevCoord_SOA
**       - rcpps isn't accurate enough for calculating 1/w!
**  16   Napalm    1.15        02/10/00 Scott Kephart   Data structure cleanup for
**       SOA.H -- we're unionized now!
**  15   Napalm    1.14        02/10/00 Bob Johnston    Cleaned up swizzling and
**       added simple prefetching
**  14   Napalm    1.13        02/10/00 Bob Johnston    Added better clipping
**       support for the test code.  It will now use HAL assembly when okay.
**  13   Napalm    1.12        02/09/00 Scott Kephart   Fixed clipping bug
**  12   Napalm    1.11        02/09/00 Scott Kephart   Fix TLBUF overwrite bug 
**  11   Napalm    1.10        02/09/00 Scott Kephart   Get some triangles on the
**       screen!
**  10   Napalm    1.9         02/08/00 Scott Kephart   Turn on test code. 
**  9    Napalm    1.8         02/08/00 Scott Kephart   Really simple device
**       coordinate transform
**  8    Napalm    1.7         02/07/00 Scott Kephart   Added clip check code
**  7    Napalm    1.6         02/07/00 Bob Johnston    Added soatest.c for the
**       validation of the Software T&L HAL's fastpath.  soatest.c is a temporary
**       file that contains test functions for validating the SIMD T&L  
**  6    Napalm    1.5         02/07/00 Bob Johnston    Added De-swizzle from Temp
**       SOA to TLBN vertex buffers, clipped and non-clipped
**  5    Napalm    1.4         02/04/00 Bob Johnston    Changes to support User
**       Memory vertex buffers for the fastpath.  Also cleaned up the vertex
**       processing loops with neater macros for better readablity.
**  4    Napalm    1.3         02/02/00 Bob Johnston    Fixed missed assignment to
**       SOAFVF lpvData and wrong pointer advancement
**  3    Napalm    1.2         02/01/00 Scott Kephart   More lighting changes.
**       Better SSE matrix multiply code.
**  2    Napalm    1.1         01/28/00 Scott Kephart   
**  1    Napalm    1.0         01/28/00 Scott Kephart   
** $
 * 
 * 6     1/27/00 12:43a Skephart
 * New changes from Bob, Joe, Scott
 * 
 * 5     1/25/00 11:47a Skephart
 * Merge from BobJ 
 * 
 * 4     1/24/00 10:40p Skephart
 * Skeleton for SOA T&L
 * 
 * 1     1/19/00 9:49p Skephart
 * Beginnings of SOA T&L code
*/


#include "precomp.h"

#if( DX >= 7 )
#ifdef TnL_HAL
#if defined(VCPP)

#ifndef WINNT
#include <d3dhal.h>
#include "d6fvf.h"
#include "fxglobal.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "fifomgr.h"
#include "d3tri.h"
#include "d6global.h"
#include "d3contxt.h"
#endif

#include "dxins.h"


//********************
// DEFINES and MACROS
//********************


//#define VB_DEBUG 1
//#define PROFILE_VB_ORDER 1



// Vertex loop processing macros
#define GRAB_NEXT_INDEX_VB( vIdx, vert, page, bit )  { \
        vIdx = (DWORD) pIndx[vert];  \
        vIdx += vStart;  \
        page = vIdx>>7;           \
        bit = (vIdx>>2) % 32;  }

#define GET_SOA_PTR( vIdx )  \
           (DWORD *)((LPBYTE)pRc->tl.SOAFVF.lpvData + ((vIdx>>2) * pRc->tl.SOAFVF.dwStride))

#define SOA_IDX( vIdx )  (vIdx & 0xfffffffc)

// use texture for flag
#define SOA_GROUP_NOT_TRANSFORMED( pVert )  ( (*(DWORD*)(&pVert->tex[0].s)) == TL_VERT_NOT_XFORMED )
#define SOA_GROUP_NOT_LIT( pVert )  		( (*(DWORD*)(&pVert->tex[0].s)) == TL_VERT_XFORMED_NOT_LIT )

// use wfbi for flag
#define K3D_GROUP_NOT_TRANSFORMED( pVert )  ( (*(DWORD*)(&pVert->wfbi)) == TL_VERT_NOT_XFORMED )
#define K3D_GROUP_NOT_LIT( pVert )  		( (*(DWORD*)(&pVert->wfbi)) == TL_VERT_XFORMED_NOT_LIT )




//**********************
// Function Prototypes
//**********************
void SetupTL_SOATMP(RC *pRc);
void SetupTL_K3DTMP(RC *pRc);


//**********************
// Static Data
//**********************
#ifdef DEBUG
static DWORD TnlStats_VertsSentToDriver=0;
static DWORD TnlStats_VertsXfrmd=0;
static DWORD TnlStats_VertsXfrmdAndLit=0;
static DWORD TnlStats_TotalTrisSentToDriver=0;
#endif


#ifdef PROFILE_VB_ORDER
//**********************
// Profiling 
//**********************

#define NUM_BINS 100
#define MAXNUMVERTICES  4096
#define MAX_SSE_GROUPS  (MAXNUMVERTICES>>7)

typedef struct __SOA_IDX_DAT
{
    DWORD new_soa;
    DWORD num_tris;
} SOA_IDX_DAT;


typedef struct __SOA_IDX_STATS
{
    DWORD count_prims;      // #of primitive calls
    DWORD ooo_flag;         // Flag to indicate if any of the verts in the current buffer are ooo.
    DWORD pref_count;       // #of SOA groups to prefetch
    DWORD min_back;         // minimum distance for backwards (already seen) access
    DWORD max_back;         // maximum distance for backwards (already seen) access
    DWORD dist_back;        // sum of distance backwards
    DWORD count_back;       // number of backwards (seen) accesses
    DWORD count_curr;       // count of accesses to the current SOA
    float avg_back;         // average count backwards
    float avg_back_dist;    // average backwards distance
    DWORD min_fwd;          // minimum out of order unseen forward distance
    DWORD max_fwd;          // maximum out of order unseen forward distance
    DWORD dist_fwd;         // sum of forward distance 
    DWORD count_fwd;        // number of forward (out of order) accesses
    float avg_fwd;          // average # of out of order forward accesses
    float avg_fwd_dist;     // average distance of out of order forward accesses
    DWORD ooo_fwd;          // Did any out of order forward accesses happen
    DWORD ooo_back;         // Did any backwards out of order accesses happen
    DWORD count_soaA;       // number of times new SOA group was needed for vertex A
    DWORD count_soaB;       // number of times new SOA group was needed for vertex B
    DWORD count_soaC;       // number of times new SOA group was needed for vertex C
    float pct_soaA;         // % of times new SOA group was needed for VA
    float pct_soaB;         // % of times new SOA group was needed for VB
    float pct_soaC;         // % of times new SOA group was needed for VC
    float pct_back4;        // percentage of backwards accesses within 4 SOA groups
    float pct_back8;        // percentage of backwards accesses within 8 SOA groups
    float pct_back16;       // percentage of backwards accesses within 16 SOA groups
    float pct_back21;       // percentage of backwards accesses within 21 SOA groups
    float pct_back32;       // percentage of backwards accesses within 32 SOA groups
    SOA_IDX_DAT *tri_list;
    float back_bin[NUM_BINS];   // Count of backwards references sorted by distance.
} SOA_IDX_STATS;


#define IS_SOA_GROUP_SEEN( page, bit )  (dwSSEIdxSeenBitfield[page] & ( 1 << bit))
#define SET_SOA_GROUP_SEEN( page, bit ) dwSSEIdxSeenBitfield[page] |= ( 1 << bit) 


// the global variable profctrl controls the accumulation and display of output
// profctrl = 0 means reset all accumulated data and start accumulating new data. 
// profctrl = 2 means to dump all currently accumulated data, and reset back to zero.

// The way you use this is set profctrl to zero in the debugger. Run the benchmark. 
// After the benchmark run ends, break into the debugger, and set profctrl = 2. Then start the benchmark again.
// hit the escape key to terminate the benchmark, and save the log file in the winice symbol loader app. 
// When you want to start profiling again, simply set profctrl to 0 prior to running the app.
// 

DWORD profctrl = 0;

// the global disp_flags controls what you want to display / accumulate during profiling
// data for multiple primitives (i.e. over an entire run of a benchmark) is always accumulated and displayed.
// bit 0 means display vertex buffers that are out of order (SOA groups and # of triangles)
// bit 1 means display all vertex buffers (SOA groups and # of triangles) (Well, really, it prints 1% of them)

DWORD disp_flags = 0;

SOA_IDX_STATS tri_stat;
SOA_IDX_STATS tri_stat_curr;
DWORD dwSSEIdxSeenBitfield[MAX_SSE_GROUPS];
SOA_IDX_DAT tri_list[MAXNUMVERTICES*3];

void gather_vertex_buffer_stats(DWORD tri_count, WORD *pIndx, DWORD vStart);
void AccumulateIndexStats(DWORD tri_count, WORD *pIndx, DWORD vStart, SOA_IDX_STATS *pTriStat);
void InitIndexStats(SOA_IDX_STATS *);
void SummarizeIndexStats(SOA_IDX_STATS *pTriStat);
void Display_Index_Stats_Results(SOA_IDX_STATS *pTriStat, int flag);
#endif // PROFILE_VB_ORDER




/*-------------------------------------------------------------------
Function Name:  AllocateSOAFVF
Description:    Creates the SOAFVF buffer for each new D3DFVF 
                Vertex Buffer
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
void AllocateSOAFVF( RC *pRc )
{
    DWORD VertCnt; 
    LPVBSURFACEDATA lpVBSD = pRc->tl.lpVBSurfData;
    NT9XDEVICEDATA *ppdev = pRc->ppdev;
    DWORD dwSize;
    DWORD numVecs;

    // BobJ 3/4/2000 -- The number of vertices handed down by DP2
    // may not be telling the truth with how many vertices are actually
    // the the VB.  3DWinMark demonstrates this
    //VertCnt = pRc->tl.dwNumVertices;      //Number of vertices in the VB

    // A better way is to look at the size of the memory allocated by the 
    // app for the InFVF and divide it by the size of the stride.
    //VertCnt = (lpVBSD->dwSrcAlignSize / pRc->tl.InFVF.dwStride);


    // Should have been calculated at ProcPrim fast path check
    VertCnt = lpVBSD->dwNumVerts;

    // need to calculate the even groups of four vertices (*4). 
    // add 1 extra if not an even power of 16
	// numVecs = # of SOA groups
    numVecs = ((VertCnt/16) + ((VertCnt%16) ? 1 : 0)) * 4;

    // Get the needed size of the optimized SOA VB
    dwSize = ((numVecs * pRc->tl.SOAFVF.dwStride) +31);

    // Allocate the Optimized Vertex Buffer
    lpVBSD->pOptAllocAddr = (LPVOID) DXMALLOC(dwSize);
    if (!lpVBSD->pOptAllocAddr)
    {
      D3DPRINT( 0, "Failure to Allocate an Optimized Vertex Buffer!");
    }
    // Need to fillout the rest of the Optimized buffer values
    lpVBSD->pOptAlignAddr = (LPVOID)(((ULONG_PTR)lpVBSD->pOptAllocAddr + 31 ) & ~31);
    lpVBSD->dwOptAllocSize = dwSize;
    lpVBSD->dwOptAlignSize = dwSize - ((DWORD)lpVBSD->pOptAlignAddr - (DWORD)lpVBSD->pOptAllocAddr);

    // Check to see if OVER allocation was too big for # vecs
    if( (numVecs * pRc->tl.SOAFVF.dwStride) <= lpVBSD->dwSrcAlignSize )
      lpVBSD->dwNumVecs = numVecs;  // It's safe
    else
      lpVBSD->dwNumVecs = numVecs - 1; // too big, back it off an SOA GROUP

}

#ifdef VB_DEBUG
/*-------------------------------------------------------------------
Function Name:  ChecksumVB
Description:    Creates a checksum for the VB Src buffer
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
DWORD ChecksumVB( LPDWORD lpSrcBuff, DWORD dwsizeb  )
{
  DWORD dwRetVal = 0;
  DWORD dwsized = dwsizeb>>2;
  DWORD i;

  for( i = 0; i < dwsized; i++)
  {
     dwRetVal += *lpSrcBuff++;
  }
  return dwRetVal;

}
#endif


/*-------------------------------------------------------------------
Function Name:  ClipCullCheckKNI
Description:    Returns non-zero if the triangle is can be trivialy 
        rejected or back-faced
Parameters:     lpTLBN *pA, *pB, *pC - pointers to the vertices
                RC *pRc - pointer to the rendering context
Information:    
Return:         ZERO/NON-ZERO


 int ClipCullCheckKNI()
    case D3DCULL_NONE:      pRc->cullMask = 0xFFFFFFFF;
    case D3DCULL_CW:        pRc->cullMask = 0x80000000;
    default:                pRc->cullMask = 0x00000000;

    DWORD ClipAll = ((pA->clip_code & pB->clip_code & pC->clip_code)) ? 1 : 0;

  DWORD FrontBackClip = (! ((pA->clip_code | pB->clip_code | pC->clip_code) & (TLCLIP_FRONT | TLCLIP_BACK))) ? 0xFFFFFFFF : 0;
  pRc->cullMask |= FrontBackClip;

    float area = (((pA->x-pB->x)*(pB->y-pC->y)) - ((pA->y-pB->y)*(pB->x-pC->x)));
    DWORD sign = AS_INT32(area) & 0x80000000L;
    DWORD Cull = ((sign ^ pRc->cullMask) == 0x80000000L) ? 1 : 0;

  return (Cull | ClipAll);

-------------------------------------------------------------------*/
__inline int ClipCullCheckKNI(RC *pRc, TLBN *pA, TLBN *pB, TLBN *pC)
{
  __asm {
  mov       edx, pB
  mov       eax, pA
  pcmpeqd   mm6, mm6                // mm6 = ffffffff fffffffff
  mov       ecx, pC

  movlps    xmm6, [edx]TLBN.x  		// xmm6 = ---- ---- vb.y vb.x
  pxor      mm7, mm7                // mm7 = 00000000 00000000
  movhps    xmm6, [eax]TLBN.x  		// xmm6 = va.y va.x vb.y vb.x
  movlps    xmm7, [ecx]TLBN.x  		// xmm7 = ---- ---- vc.y vc.x

  movlhps   xmm7, xmm6              // xmm7 = vb.y vb.x vc.y vc.x
  movd      mm1, [edx]TLBN.clip_code // mm1 = vBcc

  subps     xmm6, xmm7				// xmm6 = va.y-vb.y va.x-vb.x vb.y-vc.y vb.x-vc.x
  movd      mm0, [eax]TLBN.clip_code // mm0 = vAcc
  mov       edx, (TLCLIP_FRONT | TLCLIP_BACK)
  movd      mm2, [ecx]TLBN.clip_code // mm2 = vCcc

  movaps    xmm7, xmm6          // xmm7 = aby  abx  bcy  bcx
  movq      mm3, mm1            // mm3 = vBcc
  shufps    xmm6, xmm6, 0x5f    // xmm6 = bcy  bcy  aby  aby
  mov       eax, pRc

  mulps     xmm6, xmm7          // xmm6 = xxxxxxx abx*bcy xxxxxxx aby*bcx
  movd      mm4, edx            // mm4 = (TLCLIP_FRONT | TLCLIP_BACK)
  por       mm1, mm0            // mm1 = vAcc | vBcc
  movd      mm5, [eax]RC.cullMask   // mm5 = cull-mask
  por       mm1, mm2            // mm1 = vAcc | vBcc | vCcc
  pand      mm1, mm4            // mm1 = (vAcc | vBcc | vCcc) & (TLCLIP_FRONT | TLCLIP_BACK) = FrontBackClip

  pcmpgtd   mm1, mm7            // mm1 = (FrontBackClip != 0) ? -1 : 0
  pand      mm0, mm2            // mm0 = vAcc & vCcc

  movhlps   xmm7, xmm6          // xmm7 = xxxxxxx xxxxxxx xxxxxxx abx*bcy
  pand      mm0, mm3            // mm0 = vAcc & vBcc & vCcc = ClipAll

  subss     xmm7, xmm6          // xmm7 = abx*bcy-aby*bcx     +=CW, -=CCW
  por       mm1, mm5            // mm1 = (FrontBackClip != 0) ? -1 : cull-mask  (-1 means D3DCULL_NONE)
  pslld     mm6, 31             // mm6 = 80000000 80000000

  cvtps2pi  mm2, xmm7           // mm2 = (abx*bcy - aby*bcx)
  pcmpgtd   mm0, mm7            // mm0 = (ClipAll != 0) ? -1 : 0

  pand      mm2, mm6            // mm2 = sign(abx*bcy - aby*bcx)
  pxor      mm2, mm1            // mm2 = sign(abx*bcy - aby*bcx) ^ cull-mask
  pcmpeqd   mm2, mm6            // mm2 = (sign(abx*bcy - aby*bcx) ^ cull-mask) == 80000000) ? -1 : 0
  por       mm0, mm2            // mm0 = (sign(abx*bcy - aby*bcx) ^ cull-mask) == cull-mask) || (ClipAll != 0) ? -1 : 0
  movd      eax, mm0			// eax = (sign(abx*bcy - aby*bcx) ^ cull-mask) == cull-mask) || (ClipAll != 0) ? -1 : 0
  emms
  }
}


/*-------------------------------------------------------------------
Function Name:  CullCheckKNI
Description:    returns TRUE if the triangle is back-facing
Parameters:     TLBN *pA, *pB, *pC - pointers to the vertices
                RC *pRc - pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
__inline int CullCheckKNI(RC *pRc, TLBN *pA, TLBN *pB, TLBN *pC)
{
  __asm {
  mov   edx, pB
  mov   eax, pA
  mov   ecx, pC

  movlps  xmm6, [edx]TLBN.x  // xmm6 = ---- ---- vb.y vb.x
  movhps  xmm6, [eax]TLBN.x  // xmm6 = va.y va.x vb.y vb.x
  movlps  xmm7, [ecx]TLBN.x  // xmm7 = ---- ---- vc.y vc.x
  movlhps xmm7, xmm6         // xmm7 = vb.y vb.x vc.y vc.x
  subps xmm6, xmm7           // xmm6 = va.y-vb.y va.x-vb.x vb.y-vc.y vb.x-vc.x
  mov   edx, [pRc]
  mov   eax, 1

  movaps  xmm7, xmm6       // xmm7 = aby  abx  bcy  bcx
  shufps  xmm6, xmm6, 0x5f // xmm6 = bcy  bcy  aby  aby
  mov   ecx, [edx]RC.cullMask // edx = cullMask
  mulps xmm6, xmm7         // xmm7 = abx*bcy   aby*bcx
  and   eax, ecx           // cull_none ? 1 : 0
  shr   ecx, 31            // bit 0 = cull direction

  movhlps xmm7, xmm6       // xmm6 = xxxxxxx   abx*bcy
  subss xmm7, xmm6         // xmm7 = abx*bcy-aby*bcx
  xor   eax, 1             // cull_none ? 0 : 1

  movmskps edx, xmm7       // edx.bit0 = sign(abx*bcy-aby*bcx)
  xor   edx, ecx           // sign ^  cullMask
  and   eax, edx           // handles "cull_none" and masks off the rest of the bits
  // jnz cull
  } 
}  


// Returns 0x80000000 for culled tri's, 0 for non-culled tries
// The C version of this is actually faster
__inline int CullCheckK3D(DWORD cullMask, TLBN *pA, TLBN *pB, TLBN *pC)
{
  __asm {
  mov		eax, pA
  mov		edx, pB
  mov		ecx, pC

  movq		mm0, [eax]TLBN.x	//	va.y		va.x
  movq		mm1, [edx]TLBN.x	//	vb.y		vb.x
  movq		mm2, [ecx]TLBN.x	//	vc.y		vc.x
  pfsub		mm0, mm1			//	va.y-vb.y	va.x-vb.x
  movq		mm4, [TL_sign_mask_low]	// 00000000	80000000
  pfsub		mm1, mm2			//	vb.y-vc.y	vb.x-vc.x

  movd		mm5, [cullMask]
  movq		mm6, mm5			// cullMask
  pslld		mm5, 31				// D3DCULL_NONE ? 80000000 : 00000000
  pxor		mm5, mm4			// D3DCULL_NONE ? 00000000 : 80000000

  // 3DNow:
  movq		mm3, mm0			//	aby			abx
  psrlq		mm0, 32				//	0			aby
  psllq		mm3, 32				//	abx			0
  por		mm0, mm3			//	abx			aby

  //ATHLON: pshufw    mm0, mm0, 0x1eh     //	abx			aby

  pxor		mm1, mm4			//	bcy			-bcx
  pfmul		mm0, mm1			//	abx*bcy		-aby*bcx
  pfacc		mm0, mm0			//	  abx*bcy - aby*bcx		+=CW, -=CCW
  pxor		mm0, mm6			//	+=CW:	cullmode=D3DCULL_CW, bit31 = ? 1 : 0
								//	+=CCW:	cullmode=D3DCULL_CW, bit31 = ? 0 : 1
  pand		mm0, mm5			//	D3DCULL_NONE ? bit31 = 0 : unchanged
  movd		eax, mm0			//	cull ? 1 : 0
  femms
  }
}  


__inline int CullCheck_C(DWORD cullMask, TLBN *pA, TLBN *pB, TLBN *pC)
{
	float area;
	DWORD sign;
    area = (((pA->x-pB->x)*(pB->y-pC->y)) - ((pA->y-pB->y)*(pB->x-pC->x)));
    sign = AS_INT32(area) & 0x80000000L;
    return ((sign ^ cullMask) == 0x80000000L) ? 1 : 0;
}  



/*-------------------------------------------------------------------
Function Name:  FP_IndexedTriangleList2_SOA_Split
Description:    Fastpath Indexed Triangle List 2 Code for SOA vertex Buffers
Parameters:   
Information:    This code implements indexed Triangle List 2 for optimized
                vertex buffers. This version does not perform lighting until
                after geometry and culling.
Return:         
-------------------------------------------------------------------*/
HRESULT FP_IndexedTriangleList2_SOA_Split(RC *pRc)
{
  DWORD   triCnt = pRc->tl.dwNumIndices / 3;        //Number of triangles to render
  unsigned short *pIndx =  (unsigned short *)pRc->tl.pIndices;  // Pointer to the indices
  LPVBSURFACEDATA lpVBSD = pRc->tl.lpVBSurfData;
  DWORD   vStart = pRc->tl.InFVF.dwVStart;
  DWORD   i; 
  DWORD   vAIdx, vBIdx, vCIdx, tmpIdx;
  DWORD   *pSrcVert, *pDstVert;
  lpTLBN  pA, pB, pC;
  SOA_UV  *pTex;
  DWORD   localt0 = (pRc->tl.SOAFVF.dwTexOffset/4) + (pRc->t0CoordIndex * 4); 
  DWORD   localt1 = (pRc->tl.SOAFVF.dwTexOffset/4) + (pRc->t1CoordIndex * 4); 
  DWORD   dwSoaFvfStride = pRc->tl.SOAFVF.dwStride;
  DWORD   dwSoaFvfStride_D4 = dwSoaFvfStride/4;

#ifdef DEBUG
  TnlStats_VertsSentToDriver += pRc->tl.dwNumVertices;
  TnlStats_TotalTrisSentToDriver += triCnt;
#endif


  if(pRc->tl.dwDirtyFlags & TLPV_DIRTY_VB)
  {
    // Dirty VB so mark every vert as untransformed
	DWORD dirtyVertCnt = vStart + pRc->tl.dwNumVertices;      //Number of vertices in the VB;
    lpTLBN pTLBN = (lpTLBN)pRc->tl.TLBN.lpvData;	// TLBN vertex list we're writing to
    DWORD* pFlag = (DWORD*)&pTLBN->tex;

	_asm {
		mov		edx, pFlag
		mov		ecx, dirtyVertCnt
		mov		ebx, SIZE TLBN
	    mov		eax, TL_VERT_NOT_XFORMED
		prefetcht0 [edx]
		test	ecx, ecx
		prefetcht0 [edx+ebx]
		jz		DirtyVertsFlaged
		prefetcht0 [edx+ebx*2]
		prefetcht0 [edx+(3 * SIZE TLBN)]
	  NextDirtyVert:
		mov		[edx], eax
		prefetcht0 [edx+ebx*4]
		add		edx, ebx
		dec		ecx
		jnz		NextDirtyVert
	  DirtyVertsFlaged:\
	}
#if 0
	while(dirtyVertCnt--)
	{
	  *pFlag = TL_VERT_NOT_XFORMED;
	  pFlag = (DWORD*)((DWORD)pFlag + sizeof(TLBN));
	}
#endif
  }
  else   
    D3DPRINT( 32, "Some verts already transformed but all %d flaged", pRc->tl.dwNumVertices );


#ifdef PROFILE_VB_ORDER
  gather_vertex_buffer_stats(triCnt, pIndx, vStart);
#endif //PROFILE_VB_ORDER

  // We need to check if an Optimized SOA buffer needs to be allocated
  if(!(lpVBSD->pOptAllocAddr) && (lpVBSD->dwSrcFlags & VBSURF_JUSTCREATED))
  {
    // Call to allocate the SOAFVF buffer
    AllocateSOAFVF( pRc );

    // Diddle the flags
    lpVBSD->dwSrcFlags &= ~(VBSURF_JUSTCREATED);
    lpVBSD->dwSrcFlags |= VBSURF_ACCESSED;
  } 

  // Update SOAFVF Info for the newly allocated memory
  pRc->tl.SOAFVF.lpvData = lpVBSD->pOptAlignAddr;

  // Check to see if the curent contents of the Optimized VB has been invalidated
  // by a call to the UnlockExecuteBuffer32() callback
  if(lpVBSD->dwSrcFlags & VBSURF_ACCESSED)
  {
    // Need to re-swizzle the data in the optimized buff
    SwizzleD3DtoSOA( pRc, lpVBSD->dwNumVecs);

    // Check to see if this access is becoming a bad habit
    if ( lpVBSD->dwUnlockCounter > 2 )
    {
      // This is the 3rd time, let's nip this in the bud
      // and force through the UM path next time.
      lpVBSD->dwSrcFlags |= VBSURF_NOTQUALIFY;
    }
#ifdef VB_DEBUG
    lpVBSD->dwSrcChecksum = ChecksumVB((LPDWORD)lpVBSD->pSrcAlignAddr, (DWORD)lpVBSD->dwSrcAlignSize );
    lpVBSD->dwSrcFlags |= VBSURF_VALIDCHECKSUM;
#endif

    lpVBSD->dwSrcFlags &= ~(VBSURF_ACCESSED);  // Reset the unlock just occured flag.
  } 

#ifdef VB_DEBUG
  if( lpVBSD->dwSrcChecksum != ChecksumVB((LPDWORD)lpVBSD->pSrcAlignAddr, (DWORD)lpVBSD->dwSrcAlignSize))
  {
     D3DPRINT(0, "BAD VB SRC CHECKSUM for SrcMem = %80lX, size = %80lX", 
                              lpVBSD->pSrcAlignAddr, lpVBSD->dwSrcAlignSize );
  }
#endif

  // Setup our renderer
  TL_RENDER_Setup ( pRc );

  // Initialize some varables in TL_SOATMP
  SetupTL_SOATMP( pRc );


  // Advance the index pointer to the first real vertex index in the list
  pIndx++; 

  if (pRc->tl.dwTLState & TLPV_DOCLIPPING)
  {
    //***********************
    // CLIPPING RENDER LOOP
    //***********************
	DWORD dwClipMask = TLCLIP_LEFT  | TLCLIP_RIGHT | TLCLIP_TOP | TLCLIP_BOTTOM  | TLCLIP_FRONT | TLCLIP_BACK | TLCLIP_USERPLANES_ALL; 
	if (pRc->tl.dwTLState & TLPV_GUARDBAND) 
	  dwClipMask = TLCLIPGB_LEFT | TLCLIPGB_RIGHT | TLCLIPGB_TOP | TLCLIPGB_BOTTOM | TLCLIP_FRONT | TLCLIP_BACK | TLCLIP_USERPLANES_ALL; 

    // Check States here on a per primitive basis and assign the best local function pointer to call on
    if(pRc->state & STATE_REQUIRES_WBUFFER)
      pRc->tl.Xform_Deswiz4_DevCoord_SOA_GEOM_Funct = XformDeswiz4_DevCoord_SOA_GEOM_WBuff_Clipped;
    else
      pRc->tl.Xform_Deswiz4_DevCoord_SOA_GEOM_Funct = XformDeswiz4_DevCoord_SOA_GEOM_NoWBuff_Clipped;  

    // For each Triangle in the list
    for (i=0; i<triCnt; i++, pIndx +=3 )
    {
      // For each vertex in the triangle
	  //_mm_prefetch((char*) pIndx + 32 , _MM_HINT_NTA); 
      Prefetch_SSE_NTA((char*) pIndx + 32);

      // VERTEX A
	  vAIdx = vStart + (DWORD) pIndx[0];
      pA = (lpTLBN)((LPBYTE)pRc->tl.TLBN.lpvData + (vAIdx * TLBN_SIZE));
      if (SOA_GROUP_NOT_TRANSFORMED( pA ))
      {
        tmpIdx = vAIdx & ~0xf;
        pSrcVert = (DWORD*)((LPBYTE)pRc->tl.SOAFVF.lpvData + (tmpIdx * dwSoaFvfStride_D4));
        pDstVert = (DWORD*)((DWORD)pRc->tl.TLBN.lpvData + (tmpIdx * TLBN_SIZE));
		pRc->tl.Xform_Deswiz4_DevCoord_SOA_GEOM_Funct(pRc, pSrcVert, pDstVert);
#ifdef DEBUG
		TnlStats_VertsXfrmd += 16;
#endif
      }

      // VERTEX B
	  vBIdx = vStart + (DWORD) pIndx[1];
      pB = (lpTLBN)((LPBYTE)pRc->tl.TLBN.lpvData + (vBIdx * TLBN_SIZE));
      if (SOA_GROUP_NOT_TRANSFORMED( pB ))
      {
        tmpIdx = vBIdx & ~0xf;
        pSrcVert = (DWORD*)((LPBYTE)pRc->tl.SOAFVF.lpvData + (tmpIdx * dwSoaFvfStride_D4));
        pDstVert = (DWORD*)((DWORD)pRc->tl.TLBN.lpvData + (tmpIdx * TLBN_SIZE));
		pRc->tl.Xform_Deswiz4_DevCoord_SOA_GEOM_Funct(pRc, pSrcVert, pDstVert);
#ifdef DEBUG
		TnlStats_VertsXfrmd += 16;
#endif
      }

      // VERTEX C
	  vCIdx = vStart + (DWORD) pIndx[2];
      pC = (lpTLBN)((LPBYTE)pRc->tl.TLBN.lpvData + (vCIdx * TLBN_SIZE));
      if (SOA_GROUP_NOT_TRANSFORMED( pC ))
      {
        tmpIdx = vCIdx & ~0xf;
        pSrcVert = (DWORD*)((LPBYTE)pRc->tl.SOAFVF.lpvData + (tmpIdx * dwSoaFvfStride_D4));
        pDstVert = (DWORD*)((DWORD)pRc->tl.TLBN.lpvData + (tmpIdx * TLBN_SIZE));
		pRc->tl.Xform_Deswiz4_DevCoord_SOA_GEOM_Funct(pRc, pSrcVert, pDstVert);
#ifdef DEBUG
		TnlStats_VertsXfrmd += 16;
#endif
      }

      //************************************
      // Cull and Trivial Reject Clip Check
      //************************************
      if ( ClipCullCheckKNI(pRc, pA, pB, pC)) continue;

      //*************************
      // Light
      //*************************
      // VERTEX A
      if (SOA_GROUP_NOT_LIT( pA ))
      {
        tmpIdx = vAIdx & ~0x3;
        pSrcVert = (DWORD*)((LPBYTE)pRc->tl.SOAFVF.lpvData + (tmpIdx * dwSoaFvfStride_D4));
        FP_Light_4Vert_SOA( pRc, pSrcVert );
        Xform_DevColor_SOA(pRc);
        pTex = (SOA_UV*)(pSrcVert + pRc->tl.InFVF.dwTexOffset);
		Xform_DeSwiz_DevCoord_SOA_TEX( pRc, tmpIdx, pTex );
#ifdef DEBUG
        TnlStats_VertsXfrmdAndLit += 4;
#endif
      }

      // VERTEX B
      if (SOA_GROUP_NOT_LIT( pB ))
      {
        tmpIdx = vBIdx & ~0x3;
        pSrcVert = (DWORD*)((LPBYTE)pRc->tl.SOAFVF.lpvData + (tmpIdx * dwSoaFvfStride_D4));
        FP_Light_4Vert_SOA( pRc, pSrcVert );
        Xform_DevColor_SOA(pRc);
        pTex = (SOA_UV*)(pSrcVert + pRc->tl.InFVF.dwTexOffset);
		Xform_DeSwiz_DevCoord_SOA_TEX( pRc, tmpIdx, pTex );
#ifdef DEBUG
        TnlStats_VertsXfrmdAndLit += 4;
#endif
      }

      // VERTEX C
      if (SOA_GROUP_NOT_LIT( pC ))
      {
        tmpIdx = vCIdx & ~0x3;
        pSrcVert = (DWORD*)((LPBYTE)pRc->tl.SOAFVF.lpvData + (tmpIdx * dwSoaFvfStride_D4));
        FP_Light_4Vert_SOA( pRc, pSrcVert );
        Xform_DevColor_SOA(pRc);
        pTex = (SOA_UV*)(pSrcVert + pRc->tl.InFVF.dwTexOffset);
		Xform_DeSwiz_DevCoord_SOA_TEX( pRc, tmpIdx, pTex );
#ifdef DEBUG
        TnlStats_VertsXfrmdAndLit += 4;
#endif
      }


      //*************************
      // Call on CLIPPING 
      // Rendering code 
      // to output this triangle
      //*************************
      if(!((pA->clip_code | pB->clip_code | pC->clip_code) & dwClipMask))
        pRc->tl.KniRC.RenderTriFunct_NoClip ( &pRc->tl.KniRC, pA, pB, pC );	// No clip planes intersected -> Trivial Accept
      else
	  {
        // At least one vertex has a clip_code bit set
		D3DVERTEX pVertIn[3];
		SOA_XYZ *pTlbnIn;

		// build the non-swizzled vertex (just x,y,z)
		pTlbnIn = (SOA_XYZ*) ( (LPBYTE)pRc->tl.SOAFVF.lpvData + ((vAIdx>>2) * dwSoaFvfStride) + ((vAIdx & 3) * sizeof(D3DVALUE)) );
		pVertIn[0].x = pTlbnIn->x.f[0];		pVertIn[0].y = pTlbnIn->y.f[0];		pVertIn[0].z = pTlbnIn->z.f[0];
		pTlbnIn = (SOA_XYZ*) ( (LPBYTE)pRc->tl.SOAFVF.lpvData + ((vBIdx>>2) * dwSoaFvfStride) + ((vBIdx & 3) * sizeof(D3DVALUE)) );
		pVertIn[1].x = pTlbnIn->x.f[0];		pVertIn[1].y = pTlbnIn->y.f[0];		pVertIn[1].z = pTlbnIn->z.f[0];
		pTlbnIn = (SOA_XYZ*) ( (LPBYTE)pRc->tl.SOAFVF.lpvData + ((vCIdx>>2) * dwSoaFvfStride) + ((vCIdx & 3) * sizeof(D3DVALUE)) );
		pVertIn[2].x = pTlbnIn->x.f[0];		pVertIn[2].y = pTlbnIn->y.f[0];		pVertIn[2].z = pTlbnIn->z.f[0];

    	TL_RENDER_Default_Clip( pRc, pA, pB, pC, &pVertIn[0], &pVertIn[1], &pVertIn[2] ); 
	  }

    }

  }
  else
  {
    //***********************
    // NON-CLIPED RENDER LOOP
    //***********************

    // Check States here on a per primitive basis and assign the best local function pointer to call on
    if(pRc->state & STATE_REQUIRES_WBUFFER)
      pRc->tl.Xform_Deswiz4_DevCoord_SOA_GEOM_Funct = XformDeswiz4_DevCoord_SOA_GEOM_WBuff;
    else
      pRc->tl.Xform_Deswiz4_DevCoord_SOA_GEOM_Funct = XformDeswiz4_DevCoord_SOA_GEOM_NoWBuff;  

    // For each Triangle in the list
    for (i=0; i<triCnt; i++, pIndx +=3 )
    {
      // For each vertex in the triangle
      //_mm_prefetch((char*) pIndx + 32 , _MM_HINT_NTA); 
      Prefetch_SSE_NTA((char*) pIndx + 32);

      // VERTEX A
	  vAIdx = vStart + (DWORD) pIndx[0];
      pA = (lpTLBN)((LPBYTE)pRc->tl.TLBN.lpvData + (vAIdx * TLBN_SIZE));
      if (SOA_GROUP_NOT_TRANSFORMED( pA ))
      {
        tmpIdx = vAIdx & ~0xf;
        pSrcVert = (DWORD*)((LPBYTE)pRc->tl.SOAFVF.lpvData + (tmpIdx * dwSoaFvfStride_D4));
        pDstVert = (DWORD*)((DWORD)pRc->tl.TLBN.lpvData + (tmpIdx * TLBN_SIZE));
		pRc->tl.Xform_Deswiz4_DevCoord_SOA_GEOM_Funct(pRc, pSrcVert, pDstVert);
#ifdef DEBUG
		TnlStats_VertsXfrmd += 16;
#endif
      }

      // VERTEX B
	  vBIdx = vStart + (DWORD) pIndx[1];
      pB = (lpTLBN)((LPBYTE)pRc->tl.TLBN.lpvData + (vBIdx * TLBN_SIZE));
      if (SOA_GROUP_NOT_TRANSFORMED( pB ))
      {
        tmpIdx = vBIdx & ~0xf;
        pSrcVert = (DWORD*)((LPBYTE)pRc->tl.SOAFVF.lpvData + (tmpIdx * dwSoaFvfStride_D4));
        pDstVert = (DWORD*)((DWORD)pRc->tl.TLBN.lpvData + (tmpIdx * TLBN_SIZE));
		pRc->tl.Xform_Deswiz4_DevCoord_SOA_GEOM_Funct(pRc, pSrcVert, pDstVert);
#ifdef DEBUG
		TnlStats_VertsXfrmd += 16;
#endif
      }

      // VERTEX C
	  vCIdx = vStart + (DWORD) pIndx[2];
      pC = (lpTLBN)((LPBYTE)pRc->tl.TLBN.lpvData + (vCIdx * TLBN_SIZE));
      if (SOA_GROUP_NOT_TRANSFORMED( pC ))
      {
        tmpIdx = vCIdx & ~0xf;
        pSrcVert = (DWORD*)((LPBYTE)pRc->tl.SOAFVF.lpvData + (tmpIdx * dwSoaFvfStride_D4));
        pDstVert = (DWORD*)((DWORD)pRc->tl.TLBN.lpvData + (tmpIdx * TLBN_SIZE));
		pRc->tl.Xform_Deswiz4_DevCoord_SOA_GEOM_Funct(pRc, pSrcVert, pDstVert);
#ifdef DEBUG
		TnlStats_VertsXfrmd += 16;
#endif
      }

      //*************************
      // Cull Check
      //*************************
      if (CullCheckKNI(pRc, pA, pB, pC)) continue;

      //*************************
      // Light
      //*************************

      // VERTEX A
      if (SOA_GROUP_NOT_LIT( pA ))
      {
        tmpIdx = vAIdx & ~0x3;
        pSrcVert = (DWORD*)((LPBYTE)pRc->tl.SOAFVF.lpvData + (tmpIdx * dwSoaFvfStride_D4));
        FP_Light_4Vert_SOA( pRc, pSrcVert );
        Xform_DevColor_SOA(pRc);
        pTex = (SOA_UV*)(pSrcVert + pRc->tl.InFVF.dwTexOffset);
		Xform_DeSwiz_DevCoord_SOA_TEX( pRc, tmpIdx, pTex );
#ifdef DEBUG
        TnlStats_VertsXfrmdAndLit += 4;
#endif
      }

      // VERTEX B
      if (SOA_GROUP_NOT_LIT( pB ))
      {
        tmpIdx = vBIdx & ~0x3;
        pSrcVert = (DWORD*)((LPBYTE)pRc->tl.SOAFVF.lpvData + (tmpIdx * dwSoaFvfStride_D4));
        FP_Light_4Vert_SOA( pRc, pSrcVert );
        Xform_DevColor_SOA(pRc);
        pTex = (SOA_UV*)(pSrcVert + pRc->tl.InFVF.dwTexOffset);
		Xform_DeSwiz_DevCoord_SOA_TEX( pRc, tmpIdx, pTex );
#ifdef DEBUG
        TnlStats_VertsXfrmdAndLit += 4;
#endif
      }

      // VERTEX C
      if (SOA_GROUP_NOT_LIT( pC ))
      {
        tmpIdx = vCIdx & ~0x3;
        pSrcVert = (DWORD*)((LPBYTE)pRc->tl.SOAFVF.lpvData + (tmpIdx * dwSoaFvfStride_D4));
        FP_Light_4Vert_SOA( pRc, pSrcVert );
        Xform_DevColor_SOA(pRc);
        pTex = (SOA_UV*)(pSrcVert + pRc->tl.InFVF.dwTexOffset);
		Xform_DeSwiz_DevCoord_SOA_TEX( pRc, tmpIdx, pTex );
#ifdef DEBUG
        TnlStats_VertsXfrmdAndLit += 4;
#endif
      }

      //*************************
      // Call on UN-CLIPED 
      // Rendering code 
      // to output this triangle
      //*************************
      pRc->tl.KniRC.RenderTriFunct_NoClip ( &pRc->tl.KniRC, pA, pB, pC );

    }

  }

  TL_RENDER_Cleanup ( pRc );
  pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_VB;	// this VB has been T&L'd
  __asm  emms
  return !D3D_OK;  
}  


/*-------------------------------------------------------------------
Function Name:  FP_IndexedTriangleList2_SOA_UM_Split
Description:    Fastpath Indexed Triangle List 2 Code for 
                User Memory vertices
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
HRESULT FP_IndexedTriangleList2_SOA_UM_Split(RC *pRc)
{
  DWORD   VertCnt = pRc->tl.dwNumVertices;      //Number of vertices in the VB
  DWORD   indxCnt = pRc->tl.dwNumIndices;       //Number of 16 bit indices
  DWORD   triCnt = pRc->tl.dwNumIndices / 3;        //Number of triangles to render
  unsigned short *pIndx =  (unsigned short *)pRc->tl.pIndices;  // Pointer to the indices
  DWORD   dwSize = (VertCnt + 3) * pRc->tl.InFVF.dwStride;  // Minimum SOA Buffer Size (in bytes) needed 
  NT9XDEVICEDATA *ppdev = pRc->ppdev;
  DWORD   i;
  DWORD   vAIdx, vBIdx, vCIdx;
  DWORD   *pSOA_A, *pSOA_B, *pSOA_C;
  DWORD   vCurIdx_A, vCurIdx_B, vCurIdx_C;
  lpTLBN  pA, pB, pC;
  SOA_UV  *pTex;
  DWORD   soaTxOffset = (pRc->tl.SOAFVF.dwTexOffset/4);
  DWORD   localt0 = soaTxOffset + (pRc->t0CoordIndex * 4); 
  DWORD   localt1 = soaTxOffset + (pRc->t1CoordIndex * 4); 


#ifdef DEBUG
  TnlStats_VertsSentToDriver += pRc->tl.dwNumVertices;
  TnlStats_TotalTrisSentToDriver += triCnt;
#endif

  if(pRc->tl.dwDirtyFlags & TLPV_DIRTY_VB)
  {
    // Dirty VB so mark every vert as untransformed
    lpTLBN pTLBN = (lpTLBN)pRc->tl.TLBN.lpvData;	// TLBN vertex list we're writing to
    DWORD* pFlag = (DWORD*)&pTLBN->tex;
	DWORD dirtyVertCnt = VertCnt;
	while(dirtyVertCnt--)
	{
	  *pFlag = TL_VERT_NOT_XFORMED;
	  pFlag = (DWORD*)((DWORD)pFlag + sizeof(TLBN));
	}
  }
  else   
    D3DPRINT( 32, "Some verts already transformed but all %d flaged", VertCnt );

  // Update SOAFVF Info for the newly allocated memory
  pRc->tl.SOAFVF.lpvData = pRc->tl.SOAUMBuf.alignedBuf;
  pSOA_A = GET_SOA_PTR( 0 );
  pSOA_B = GET_SOA_PTR( 4 );
  pSOA_C = GET_SOA_PTR( 8 );

  // Initialize these to and index that doesn't exist
  vCurIdx_A = vCurIdx_B = vCurIdx_C = 0xffffffff;

  // Setup our renderer
  TL_RENDER_Setup ( pRc );

  // Setup the swizzle-single functions
  SwizzleSingleD3DtoSOA_Setup( pRc );

  // Initialize some varables in TL_SOATMP
  SetupTL_SOATMP( pRc );

  // Check States here on a per primitive basis and
  // assign the best local function pointer to call on
  if(pRc->state & STATE_REQUIRES_WBUFFER)
    pRc->tl.Xform_DevCoord_SOA_GEOM_Funct = Xform_DevCoord_SOA_GEOM_WBuff;
  else
    pRc->tl.Xform_DevCoord_SOA_GEOM_Funct = Xform_DevCoord_SOA_GEOM_NoWBuff;  

  // Advance the index pointer to the fisrt real vertex index in the list
  pIndx++; 

  if (pRc->tl.dwTLState & TLPV_DOCLIPPING)
  {
    //***********************
    // CLIPPING RENDER LOOP
    //***********************
	DWORD dwClipMask = TLCLIP_LEFT  | TLCLIP_RIGHT | TLCLIP_TOP | TLCLIP_BOTTOM  | TLCLIP_FRONT | TLCLIP_BACK | TLCLIP_USERPLANES_ALL; 
	if (pRc->tl.dwTLState & TLPV_GUARDBAND) 
	  dwClipMask = TLCLIPGB_LEFT | TLCLIPGB_RIGHT | TLCLIPGB_TOP | TLCLIPGB_BOTTOM | TLCLIP_FRONT | TLCLIP_BACK | TLCLIP_USERPLANES_ALL; 

    // For each Triangle in the list
    for (i=0; i<triCnt; i++, pIndx += 3)
    {
      // For each vertex in the triangle
	  //_mm_prefetch((char*) pIndx + 32 , _MM_HINT_NTA); 
      Prefetch_SSE_NTA((char*) pIndx + 32);

      // VERTEX A
      vAIdx = (DWORD) pIndx[0];
      pA = (lpTLBN)((LPBYTE)pRc->tl.TLBN.lpvData + (vAIdx * TLBN_SIZE));
      if (SOA_GROUP_NOT_TRANSFORMED( pA ))
      {
        vCurIdx_A = vAIdx & ~0x3;
		if(vCurIdx_A + 3 <= VertCnt)
          pRc->tl.KniSwizParmsFunct (
            (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_A * pRc->tl.InFVF.dwStride)),
            (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (0 * pRc->tl.SOAFVF.dwStride)), 
            pRc->tl.InFVF.dwStride, pRc->tl.KniCpyTxtrFunct );
		else
          SwizzlePartialSingleD3DtoSOA( pRc, 
            (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_A * pRc->tl.InFVF.dwStride)),
            (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (0 * pRc->tl.SOAFVF.dwStride)), 
			VertCnt - (vAIdx & ~3));
        FP_Xform_4Vert_SOA( pRc, pSOA_A );
        pRc->tl.Xform_DevCoord_SOA_GEOM_Funct ( pRc );
        DeSwizzleSOAtoTLBNClipped_GEOM( pRc, vCurIdx_A );
#ifdef DEBUG
		TnlStats_VertsXfrmd += 4;
#endif
      }

      // VERTEX B
      vBIdx = (DWORD) pIndx[1];
      pB = (lpTLBN)((LPBYTE)pRc->tl.TLBN.lpvData + (vBIdx * TLBN_SIZE));
      if (SOA_GROUP_NOT_TRANSFORMED( pB ))
      {
        vCurIdx_B = vBIdx & ~0x3;
		if(vCurIdx_B + 3 <= VertCnt)
          pRc->tl.KniSwizParmsFunct (
            (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_B * pRc->tl.InFVF.dwStride)),
            (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (1 * pRc->tl.SOAFVF.dwStride)), 
            pRc->tl.InFVF.dwStride, pRc->tl.KniCpyTxtrFunct );
		else
          SwizzlePartialSingleD3DtoSOA( pRc, 
            (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_B * pRc->tl.InFVF.dwStride)),
            (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (1 * pRc->tl.SOAFVF.dwStride)), 
			VertCnt - (vBIdx & ~3));
        FP_Xform_4Vert_SOA( pRc, pSOA_B );
        pRc->tl.Xform_DevCoord_SOA_GEOM_Funct ( pRc );
        DeSwizzleSOAtoTLBNClipped_GEOM( pRc, vCurIdx_B );
#ifdef DEBUG
		TnlStats_VertsXfrmd += 4;
#endif
      }

      // VERTEX C
      vCIdx = (DWORD) pIndx[2];
      pC = (lpTLBN)((LPBYTE)pRc->tl.TLBN.lpvData + (vCIdx * TLBN_SIZE));
      if (SOA_GROUP_NOT_TRANSFORMED( pC ))
      {
        vCurIdx_C = vCIdx & ~0x3;
		if(vCurIdx_C + 3 <= VertCnt)
          pRc->tl.KniSwizParmsFunct (
            (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_C * pRc->tl.InFVF.dwStride)),
            (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (2 * pRc->tl.SOAFVF.dwStride)), 
            pRc->tl.InFVF.dwStride, pRc->tl.KniCpyTxtrFunct );
		else
          SwizzlePartialSingleD3DtoSOA( pRc, 
            (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_C * pRc->tl.InFVF.dwStride)),
            (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (2 * pRc->tl.SOAFVF.dwStride)), 
			VertCnt - (vCIdx & ~3));
        FP_Xform_4Vert_SOA( pRc, pSOA_C );
        pRc->tl.Xform_DevCoord_SOA_GEOM_Funct ( pRc );
        DeSwizzleSOAtoTLBNClipped_GEOM( pRc, vCurIdx_C );
#ifdef DEBUG
		TnlStats_VertsXfrmd += 4;
#endif
      }

      //************************************
      // Cull and Trivial Reject Clip Check
      //************************************
      if ( ClipCullCheckKNI(pRc, pA, pB, pC)) continue;

      //*************************
      // Light
      //*************************
      // VERTEX A
      if (SOA_GROUP_NOT_LIT( pA ))
      {
        // Incase this was transformed a long time ago
        // Re-swizzle it now to get it back into context
        if( vCurIdx_A != (vAIdx & ~0x3))
        {
          vCurIdx_A = vAIdx & ~0x3;
		  if(vCurIdx_A + 3 <= VertCnt)
            pRc->tl.KniSwizParmsFunct (
              (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_A * pRc->tl.InFVF.dwStride)),
              (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (0 * pRc->tl.SOAFVF.dwStride)), 
              pRc->tl.InFVF.dwStride, pRc->tl.KniCpyTxtrFunct );
		  else
            SwizzlePartialSingleD3DtoSOA( pRc, 
              (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_A * pRc->tl.InFVF.dwStride)),
              (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (0 * pRc->tl.SOAFVF.dwStride)), 
			  VertCnt - (vAIdx & ~3));
        }

        FP_Light_4Vert_SOA( pRc, pSOA_A );
        Xform_DevColor_SOA(pRc);

        pTex = (SOA_UV*)(pSOA_A + pRc->tl.InFVF.dwTexOffset);
		Xform_DeSwiz_DevCoord_SOA_TEX( pRc, vCurIdx_A, pTex );
#ifdef DEBUG
        TnlStats_VertsXfrmdAndLit += 4;
#endif
      }

      // VERTEX B
      if (SOA_GROUP_NOT_LIT( pB ))
      {
        // Incase this was transformed a long time ago
        // Re-swizzle it now to get it back into context
        if( vCurIdx_B != (vBIdx & ~0x3))
        {
          vCurIdx_B = vBIdx & ~0x3;
		  if(vCurIdx_B + 3 <= VertCnt)
            pRc->tl.KniSwizParmsFunct (
              (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_B * pRc->tl.InFVF.dwStride)),
              (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (1 * pRc->tl.SOAFVF.dwStride)), 
              pRc->tl.InFVF.dwStride, pRc->tl.KniCpyTxtrFunct );
		  else
            SwizzlePartialSingleD3DtoSOA( pRc, 
              (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_B * pRc->tl.InFVF.dwStride)),
              (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (1 * pRc->tl.SOAFVF.dwStride)), 
			  VertCnt - (vBIdx & ~3));
        }

        FP_Light_4Vert_SOA( pRc, pSOA_B );
        Xform_DevColor_SOA(pRc);

        pTex = (SOA_UV*)(pSOA_B + pRc->tl.InFVF.dwTexOffset);
		Xform_DeSwiz_DevCoord_SOA_TEX( pRc, vCurIdx_B, pTex );
#ifdef DEBUG
        TnlStats_VertsXfrmdAndLit += 4;
#endif
      }

      // VERTEX C
      if (SOA_GROUP_NOT_LIT( pC ))
      {
        // Incase this was transformed a long time ago
        // Re-swizzle it now to get it back into context
        if( vCurIdx_C != (vCIdx & ~0x3))
        {
          vCurIdx_C = vCIdx & ~0x3;
		  if(vCurIdx_C + 3 <= VertCnt)
            pRc->tl.KniSwizParmsFunct (
              (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_C * pRc->tl.InFVF.dwStride)),
              (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (2 * pRc->tl.SOAFVF.dwStride)), 
              pRc->tl.InFVF.dwStride, pRc->tl.KniCpyTxtrFunct );
		  else
            SwizzlePartialSingleD3DtoSOA( pRc, 
              (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_C * pRc->tl.InFVF.dwStride)),
              (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (2 * pRc->tl.SOAFVF.dwStride)), 
			  VertCnt - (vCIdx & ~3));
        }

        FP_Light_4Vert_SOA( pRc, pSOA_C );
        Xform_DevColor_SOA(pRc);

        pTex = (SOA_UV*)(pSOA_C + pRc->tl.InFVF.dwTexOffset);
		Xform_DeSwiz_DevCoord_SOA_TEX( pRc, vCurIdx_C, pTex );
#ifdef DEBUG
        TnlStats_VertsXfrmdAndLit += 4;
#endif
      }

      //*************************
      // Call on CLIPPING 
      // Rendering code 
      // to output this triangle
      //*************************
      if(!((pA->clip_code | pB->clip_code | pC->clip_code) & dwClipMask))
      {
        // No clip planes intersected
        // Trivial Accept
        pRc->tl.KniRC.RenderTriFunct_NoClip ( &pRc->tl.KniRC, pA, pB, pC );
      }
      else
      {
        // At least one vertex has a clip_code bit set
	    DWORD* pSrcData = (DWORD*)pRc->tl.InFVF.lpvData;
	    DWORD dwStrideInFVF = pRc->tl.InFVF.dwStride;
	    D3DVERTEX *pAin = (D3DVERTEX*)((DWORD)pSrcData + (vAIdx*dwStrideInFVF));
		D3DVERTEX *pBin = (D3DVERTEX*)((DWORD)pSrcData + (vBIdx*dwStrideInFVF));
		D3DVERTEX *pCin = (D3DVERTEX*)((DWORD)pSrcData + (vCIdx*dwStrideInFVF));
    	TL_RENDER_Default_Clip( pRc, pA, pB, pC, pAin, pBin, pCin ); 
      }

    }

  }
  else
  {
    //***********************
    // NON-CLIPED RENDER LOOP
    //***********************

    // For each Triangle in the list
    for (i=0; i<triCnt; i++, pIndx += 3)
    {
      // For each vertex in the triangle
	  //_mm_prefetch((char*) pIndx + 32 , _MM_HINT_NTA); 
      Prefetch_SSE_NTA((char*) pIndx + 32);

      // VERTEX A
      vAIdx = (DWORD) pIndx[0];
      pA = (lpTLBN)((LPBYTE)pRc->tl.TLBN.lpvData + (vAIdx * TLBN_SIZE));
      if (SOA_GROUP_NOT_TRANSFORMED( pA ))
      {
        vCurIdx_A = vAIdx & ~0x3;
		if(vCurIdx_A + 3 <= VertCnt)
          pRc->tl.KniSwizParmsFunct (
            (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_A * pRc->tl.InFVF.dwStride)),
            (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (0 * pRc->tl.SOAFVF.dwStride)), 
            pRc->tl.InFVF.dwStride, pRc->tl.KniCpyTxtrFunct );
		else
          SwizzlePartialSingleD3DtoSOA( pRc, 
            (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_A * pRc->tl.InFVF.dwStride)),
            (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (0 * pRc->tl.SOAFVF.dwStride)), 
			VertCnt - (vAIdx & ~3));
        FP_Xform_4Vert_SOA( pRc, pSOA_A );
        pRc->tl.Xform_DevCoord_SOA_GEOM_Funct ( pRc );
        DeSwizzleSOAtoTLBN_GEOM( pRc, vCurIdx_A );
#ifdef DEBUG
		TnlStats_VertsXfrmd += 4;
#endif
      }

      // VERTEX B
      vBIdx = (DWORD) pIndx[1];
      pB = (lpTLBN)((LPBYTE)pRc->tl.TLBN.lpvData + (vBIdx * TLBN_SIZE));
      if (SOA_GROUP_NOT_TRANSFORMED( pB ))
      {
        vCurIdx_B = vBIdx & ~0x3;
		if(vCurIdx_B + 3 <= VertCnt)
          pRc->tl.KniSwizParmsFunct (
            (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_B * pRc->tl.InFVF.dwStride)),
            (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (1 * pRc->tl.SOAFVF.dwStride)), 
            pRc->tl.InFVF.dwStride, pRc->tl.KniCpyTxtrFunct );
		else
          SwizzlePartialSingleD3DtoSOA( pRc, 
            (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_B * pRc->tl.InFVF.dwStride)),
            (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (1 * pRc->tl.SOAFVF.dwStride)), 
			VertCnt - (vBIdx & ~3));
        FP_Xform_4Vert_SOA( pRc, pSOA_B );
        pRc->tl.Xform_DevCoord_SOA_GEOM_Funct ( pRc );
        DeSwizzleSOAtoTLBN_GEOM( pRc, vCurIdx_B );
#ifdef DEBUG
		TnlStats_VertsXfrmd += 4;
#endif
      }

      // VERTEX C
      vCIdx = (DWORD) pIndx[2];
      pC = (lpTLBN)((LPBYTE)pRc->tl.TLBN.lpvData + (vCIdx * TLBN_SIZE));
      if (SOA_GROUP_NOT_TRANSFORMED( pC ))
      {
        vCurIdx_C = vCIdx & ~0x3;
		if(vCurIdx_C + 3 <= VertCnt)
          pRc->tl.KniSwizParmsFunct (
            (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_C * pRc->tl.InFVF.dwStride)),
            (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (2 * pRc->tl.SOAFVF.dwStride)), 
            pRc->tl.InFVF.dwStride, pRc->tl.KniCpyTxtrFunct );
		else
          SwizzlePartialSingleD3DtoSOA( pRc, 
            (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_C * pRc->tl.InFVF.dwStride)),
            (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (2 * pRc->tl.SOAFVF.dwStride)), 
			VertCnt - (vCIdx & ~3));
        FP_Xform_4Vert_SOA( pRc, pSOA_C );
        pRc->tl.Xform_DevCoord_SOA_GEOM_Funct ( pRc );
        DeSwizzleSOAtoTLBN_GEOM( pRc, vCurIdx_C );
#ifdef DEBUG
		TnlStats_VertsXfrmd += 4;
#endif
      }

      //*************************
      // Cull Check
      //*************************
      if (CullCheckKNI(pRc, pA, pB, pC)) continue;

      //*************************
      // Light
      //*************************

      // VERTEX A
      if (SOA_GROUP_NOT_LIT( pA ))
      {
        // Incase this was transformed a long time ago
        // Re-swizzle it now to get it back into context
        if( vCurIdx_A != (vAIdx & ~0x3))
        {
          vCurIdx_A = vAIdx & ~0x3;
		  if(vCurIdx_A + 3 <= VertCnt)
            pRc->tl.KniSwizParmsFunct (
              (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_A * pRc->tl.InFVF.dwStride)),
              (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (0 * pRc->tl.SOAFVF.dwStride)), 
              pRc->tl.InFVF.dwStride, pRc->tl.KniCpyTxtrFunct );
		  else
            SwizzlePartialSingleD3DtoSOA( pRc, 
              (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_A * pRc->tl.InFVF.dwStride)),
              (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (0 * pRc->tl.SOAFVF.dwStride)), 
			  VertCnt - (vAIdx & ~3));
        }

        FP_Light_4Vert_SOA( pRc, pSOA_A );
        Xform_DevColor_SOA(pRc);

        pTex = (SOA_UV*)(pSOA_A + pRc->tl.InFVF.dwTexOffset);
		Xform_DeSwiz_DevCoord_SOA_TEX( pRc, vCurIdx_A, pTex );
#ifdef DEBUG
        TnlStats_VertsXfrmdAndLit += 4;
#endif
      }

      // VERTEX B
      if (SOA_GROUP_NOT_LIT( pB ))
      {
        // Incase this was transformed a long time ago
        // Re-swizzle it now to get it back into context
        if( vCurIdx_B != (vBIdx & ~0x3))
        {
          vCurIdx_B = vBIdx & ~0x3;
		  if(vCurIdx_B + 3 <= VertCnt)
            pRc->tl.KniSwizParmsFunct (
              (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_B * pRc->tl.InFVF.dwStride)),
              (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (1 * pRc->tl.SOAFVF.dwStride)), 
              pRc->tl.InFVF.dwStride, pRc->tl.KniCpyTxtrFunct );
		  else
            SwizzlePartialSingleD3DtoSOA( pRc, 
              (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_B * pRc->tl.InFVF.dwStride)),
              (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (1 * pRc->tl.SOAFVF.dwStride)), 
			  VertCnt - (vBIdx & ~3));
        }

        FP_Light_4Vert_SOA( pRc, pSOA_B );
        Xform_DevColor_SOA(pRc);

        pTex = (SOA_UV*)(pSOA_B + pRc->tl.InFVF.dwTexOffset);
		Xform_DeSwiz_DevCoord_SOA_TEX( pRc, vCurIdx_B, pTex );
#ifdef DEBUG
        TnlStats_VertsXfrmdAndLit += 4;
#endif
      }

      // VERTEX C
      if (SOA_GROUP_NOT_LIT( pC ))
      {
        // Incase this was transformed a long time ago
        // Re-swizzle it now to get it back into context
        if( vCurIdx_C != (vCIdx & ~0x3))
        {
          vCurIdx_C = vCIdx & ~0x3;
		  if(vCurIdx_C + 3 <= VertCnt)
            pRc->tl.KniSwizParmsFunct (
              (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_C * pRc->tl.InFVF.dwStride)),
              (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (2 * pRc->tl.SOAFVF.dwStride)), 
              pRc->tl.InFVF.dwStride, pRc->tl.KniCpyTxtrFunct );
		  else
            SwizzlePartialSingleD3DtoSOA( pRc, 
              (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vCurIdx_C * pRc->tl.InFVF.dwStride)),
              (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (2 * pRc->tl.SOAFVF.dwStride)), 
			  VertCnt - (vCIdx & ~3));
        }

        FP_Light_4Vert_SOA( pRc, pSOA_C );
        Xform_DevColor_SOA(pRc);

        pTex = (SOA_UV*)(pSOA_C + pRc->tl.InFVF.dwTexOffset);
		Xform_DeSwiz_DevCoord_SOA_TEX( pRc, vCurIdx_C, pTex );
#ifdef DEBUG
        TnlStats_VertsXfrmdAndLit += 4;
#endif
      }


      //*************************
      // Call on UN-CLIPED 
      // Rendering code 
      // to output this triangle
      //*************************
      pRc->tl.KniRC.RenderTriFunct_NoClip ( &pRc->tl.KniRC, pA, pB, pC );

    }

  }

  TL_RENDER_Cleanup ( pRc );
  pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_VB;	// this VB has been T&L'd
  __asm  emms
  return !D3D_OK;  
}  



/*-------------------------------------------------------------------
Function Name:  FP_IndexedTriangleList2_K3D
Description:    Fastpath Indexed Triangle List 2 Code for 
                User Memory vertices
Parameters:   
Information:    There are 3 loops each for clipped and non-clipped
				primitives.
				In the first case we believe almost all verts will 
				be used by the primitive so we transform them all.
				In the second case many will be used so we mark all 
				as untransformed and only transform/light the ones 
				we need.  In this case we'll usually hit this same 
				VB again so we won't have to re-mark all the verts
				as untransformed.
				The third case is when they only use a small portion
				of the VB.  In this case we transform all 3 verts 
				for every triangle and then light if necessary.
Return:         
-------------------------------------------------------------------*/
HRESULT FP_IndexedTriangleList2_K3D(RC *pRc)
{
  DWORD   VertCnt = pRc->tl.dwNumVertices;      	//Number of vertices in the VB
  DWORD   triCnt = pRc->tl.dwNumIndices / 3;        //Number of triangles to render
  unsigned short *pIndx =  (unsigned short *)pRc->tl.pIndices;  // Pointer to the indices
  DWORD   cullMask = pRc->cullMask;
  DWORD   ClipCodeOR;

#ifdef DEBUG
  TnlStats_VertsSentToDriver += pRc->tl.dwNumVertices;
  TnlStats_TotalTrisSentToDriver += triCnt;
#endif

  // Setup our renderer
  TL_RENDER_Setup( pRc );

  // Initialize some varables in TL_SOATMP
  SetupTL_K3DTMP( pRc );

  // Advance the index pointer to the first real vertex index in the list
  pIndx++;

  if (pRc->tl.dwTLState & TLPV_DOCLIPPING)
  {
    //***********************
    // CLIPPING RENDER LOOPS
    //***********************
    lpTLBN  pTLBN = (lpTLBN)pRc->tl.TLBN.lpvData;	// TLBN vertex list we're writing to
    lpTLBN  pA, pB, pC;
    DWORD dwClipMask;

	if (pRc->tl.dwTLState & TLPV_GUARDBAND) 
	  dwClipMask = TLCLIPGB_LEFT | TLCLIPGB_RIGHT | TLCLIPGB_TOP | TLCLIPGB_BOTTOM | TLCLIP_FRONT | TLCLIP_BACK | TLCLIP_USERPLANES_ALL;
    else
	  dwClipMask = TLCLIP_LEFT | TLCLIP_RIGHT | TLCLIP_TOP | TLCLIP_BOTTOM  | TLCLIP_FRONT | TLCLIP_BACK | TLCLIP_USERPLANES_ALL;

    if ( (( pRc->tl.dwDirtyFlags & TLPV_DIRTY_VB) && (triCnt > (VertCnt/4))) ||	// new VB, most verts are used
         ((!pRc->tl.dwDirtyFlags & TLPV_DIRTY_VB) &&  pRc->tl.transformedVB) )	// we've alreay seen this one and it's already transformed
    {
	  /* 
	  *	Looks like they're using almost every vert so we'll just transform them all
	  */
	  if(pRc->tl.dwDirtyFlags & TLPV_DIRTY_VB)
	  {
	    // Dirty VB so transform every vert
	    pRc->tl.pTLK->K3dXformFnp ( pRc, (DWORD*)pRc->tl.InFVF.lpvData, pTLBN, VertCnt );
#ifdef DEBUG
		TnlStats_VertsXfrmd += VertCnt;
#endif
      }
      else   
        D3DPRINT( 32, "All verts already transformed");


      // For each Triangle in the list
	  while(triCnt--)
      {
	    pA = &pTLBN[(DWORD)pIndx[0]];
	    pB = &pTLBN[(DWORD)pIndx[1]];
	    pC = &pTLBN[(DWORD)pIndx[2]];
		// Common clip plane intersection, Trivial Reject because
		// the whole tri is outside the viewport fustrum
        if(! (pA->clip_code & pB->clip_code & pC->clip_code))
		{
          ClipCodeOR = pA->clip_code | pB->clip_code | pC->clip_code;
	      if (((ClipCodeOR & (TLCLIP_FRONT|TLCLIP_BACK)) != 0) || (CullCheck_C(cullMask, pA, pB, pC) == 0) )
		  {
#ifdef DEBUG
	        if (K3D_GROUP_NOT_LIT(pA))	TnlStats_VertsXfrmdAndLit++;
	        if (K3D_GROUP_NOT_LIT(pB))	TnlStats_VertsXfrmdAndLit++;
	        if (K3D_GROUP_NOT_LIT(pC))	TnlStats_VertsXfrmdAndLit++;
#endif
	        if (K3D_GROUP_NOT_LIT(pA))
	  	  	  FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[0], pA );
	    	if (K3D_GROUP_NOT_LIT(pB))
	  	  	  FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[1], pB );
	    	if (K3D_GROUP_NOT_LIT(pC))
	  	  	  FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[2], pC );

        	if(! (ClipCodeOR & dwClipMask))
          	  // No clip planes intersected, Trivial Accept
          	  TL_RENDER_Default_NoClip( &pRc->tl.KniRC, pA, pB, pC );
        	else
          	{ // At least one vertex has a clip_code bit set
	          DWORD* pSrcData = (DWORD*)pRc->tl.InFVF.lpvData;
	          DWORD dwStrideInFVF = pRc->tl.InFVF.dwStride;
			  D3DVERTEX *pAin = (D3DVERTEX*)((DWORD)pSrcData + ((DWORD)pIndx[0])*dwStrideInFVF);
			  D3DVERTEX *pBin = (D3DVERTEX*)((DWORD)pSrcData + ((DWORD)pIndx[1])*dwStrideInFVF);
			  D3DVERTEX *pCin = (D3DVERTEX*)((DWORD)pSrcData + ((DWORD)pIndx[2])*dwStrideInFVF);
    	  	  TL_RENDER_Default_Clip( pRc, pA, pB, pC, pAin, pBin, pCin ); 
			}
		  }
		}
	    pIndx += 3;
	  }
	  pRc->tl.transformedVB = TRUE;
      pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_VB;
    }
	else if ( (triCnt > (VertCnt/8)) || 					// a small portion of the verts are used
	    (!(pRc->tl.dwDirtyFlags & TLPV_DIRTY_VB)) )			// some of the verts are transformed
	{
	  /*
	  *	They're using some of the verts, we'll flag them all 
	  * as untransformed and transform only the ones we need.
	  */
	  DWORD* pSrcData = (DWORD*)pRc->tl.InFVF.lpvData;
	  DWORD dwStrideInFVF = pRc->tl.InFVF.dwStride;

	  if(pRc->tl.dwDirtyFlags & TLPV_DIRTY_VB)
	  {
	    // Dirty VB so mark every vert as untransformed
	    DWORD* pFlag = (DWORD*)&pTLBN->wfbi;
		DWORD dirtyVertCnt = VertCnt;
	    while(dirtyVertCnt--)
		{
		  *pFlag = TL_VERT_NOT_XFORMED;
		  pFlag = (DWORD*)((DWORD)pFlag + sizeof(TLBN));
		}
	  }
      else   
        D3DPRINT( 32, "Some verts already transformed but all %d flaged", VertCnt );


      // For each Triangle in the list
	  while(triCnt--)
      {
	    pA = &pTLBN[(DWORD)pIndx[0]];
	    pB = &pTLBN[(DWORD)pIndx[1]];
	    pC = &pTLBN[(DWORD)pIndx[2]];
#ifdef DEBUG
	    if(K3D_GROUP_NOT_TRANSFORMED(pA))		TnlStats_VertsXfrmd++;
	    if(K3D_GROUP_NOT_TRANSFORMED(pB))		TnlStats_VertsXfrmd++;
	    if(K3D_GROUP_NOT_TRANSFORMED(pC))		TnlStats_VertsXfrmd++;
#endif
	    if(K3D_GROUP_NOT_TRANSFORMED(pA))
	      pRc->tl.pTLK->K3dXformFnp( pRc, (DWORD*)((DWORD)pSrcData + ((DWORD)pIndx[0])*dwStrideInFVF), pA, 1 );
	    if(K3D_GROUP_NOT_TRANSFORMED(pB))
	      pRc->tl.pTLK->K3dXformFnp( pRc, (DWORD*)((DWORD)pSrcData + ((DWORD)pIndx[1])*dwStrideInFVF), pB, 1 );
	    if(K3D_GROUP_NOT_TRANSFORMED(pC))
	      pRc->tl.pTLK->K3dXformFnp( pRc, (DWORD*)((DWORD)pSrcData + ((DWORD)pIndx[2])*dwStrideInFVF), pC, 1 );

		// Common clip plane intersection, Trivial Reject because
		// the whole tri is outside the viewport fustrum
        if(! (pA->clip_code & pB->clip_code & pC->clip_code))
		{
          ClipCodeOR = pA->clip_code | pB->clip_code | pC->clip_code;
	      if (((ClipCodeOR & (TLCLIP_FRONT|TLCLIP_BACK)) != 0) || (CullCheck_C(cullMask, pA, pB, pC) == 0) )
		  {
#ifdef DEBUG
	        if (K3D_GROUP_NOT_LIT(pA))	TnlStats_VertsXfrmdAndLit++;
	        if (K3D_GROUP_NOT_LIT(pB))	TnlStats_VertsXfrmdAndLit++;
	        if (K3D_GROUP_NOT_LIT(pC))	TnlStats_VertsXfrmdAndLit++;
#endif
	        if (K3D_GROUP_NOT_LIT(pA))
	  	  	  FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[0], pA );
	        if (K3D_GROUP_NOT_LIT(pB))
	  	  	  FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[1], pB );
	        if (K3D_GROUP_NOT_LIT(pC))
	  	  	  FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[2], pC );

        	if(! (ClipCodeOR & dwClipMask))
          	  // No clip planes intersected, Trivial Accept
          	  TL_RENDER_Default_NoClip( &pRc->tl.KniRC, pA, pB, pC );
        	else
          	{ // At least one vertex has a clip_code bit set
	          DWORD* pSrcData = (DWORD*)pRc->tl.InFVF.lpvData;
	          DWORD dwStrideInFVF = pRc->tl.InFVF.dwStride;
			  D3DVERTEX *pAin = (D3DVERTEX*)((DWORD)pSrcData + ((DWORD)pIndx[0])*dwStrideInFVF);
			  D3DVERTEX *pBin = (D3DVERTEX*)((DWORD)pSrcData + ((DWORD)pIndx[1])*dwStrideInFVF);
			  D3DVERTEX *pCin = (D3DVERTEX*)((DWORD)pSrcData + ((DWORD)pIndx[2])*dwStrideInFVF);
    	  	  TL_RENDER_Default_Clip( pRc, pA, pB, pC, pAin, pBin, pCin ); 
			}
		  }
		}
	    pIndx += 3;
	  }
	  pRc->tl.transformedVB = FALSE;
      pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_VB;
	}
	else
	{
	  /*
	  *	Almost all of this vertex buffer won't be used.  Since flaging every 
	  * TLBN would require touching way more verts we'll bite the bullet and 
	  * transform every vert every tri.
	  */
	  DWORD* pSrcData = (DWORD*)pRc->tl.InFVF.lpvData;
	  DWORD dwStrideInFVF = pRc->tl.InFVF.dwStride;

      // For each Triangle in the list
	  while(triCnt--)
      {
	    pA = &pTLBN[(DWORD)pIndx[0]];
	    pB = &pTLBN[(DWORD)pIndx[1]];
	    pC = &pTLBN[(DWORD)pIndx[2]];
	    pRc->tl.pTLK->K3dXformFnp( pRc, (DWORD*)((DWORD)pSrcData + ((DWORD)pIndx[0])*dwStrideInFVF), pA, 1 );
	    pRc->tl.pTLK->K3dXformFnp( pRc, (DWORD*)((DWORD)pSrcData + ((DWORD)pIndx[1])*dwStrideInFVF), pB, 1 );
	    pRc->tl.pTLK->K3dXformFnp( pRc, (DWORD*)((DWORD)pSrcData + ((DWORD)pIndx[2])*dwStrideInFVF), pC, 1 );
#ifdef DEBUG
		TnlStats_VertsXfrmd += 3;
#endif

		// Common clip plane intersection, Trivial Reject because
		// the whole tri is outside the viewport fustrum
        if(! (pA->clip_code & pB->clip_code & pC->clip_code))
		{
          ClipCodeOR = pA->clip_code | pB->clip_code | pC->clip_code;
	      if (((ClipCodeOR & (TLCLIP_FRONT|TLCLIP_BACK)) != 0) || (CullCheck_C(cullMask, pA, pB, pC) == 0) )
		  {
	  	  	FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[0], pA );
	  	  	FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[1], pB );
	  	  	FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[2], pC );
#ifdef DEBUG
	        TnlStats_VertsXfrmdAndLit += 3;
#endif

        	if(! (ClipCodeOR & dwClipMask))
          	  // No clip planes intersected, Trivial Accept
          	  TL_RENDER_Default_NoClip( &pRc->tl.KniRC, pA, pB, pC );
        	else
          	{ // At least one vertex has a clip_code bit set
			  D3DVERTEX *pAin = (D3DVERTEX*)((DWORD)pSrcData + ((DWORD)pIndx[0])*dwStrideInFVF);
			  D3DVERTEX *pBin = (D3DVERTEX*)((DWORD)pSrcData + ((DWORD)pIndx[1])*dwStrideInFVF);
			  D3DVERTEX *pCin = (D3DVERTEX*)((DWORD)pSrcData + ((DWORD)pIndx[2])*dwStrideInFVF);
    	  	  TL_RENDER_Default_Clip( pRc, pA, pB, pC, pAin, pBin, pCin ); 
			}
		  }
		}
	    pIndx += 3;
	  }
	  pRc->tl.transformedVB = FALSE;
      pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VB;
	}
  } // end of clipped vertex loops
  else
  {
    //*************************
    // NON-CLIPED RENDER LOOPS
    //*************************
    lpTLBN  pTLBN = (lpTLBN) pRc->tl.TLBN.lpvData;	// TLBN vertex list we're writing to
    lpTLBN  pA, pB, pC;

    if ( (( pRc->tl.dwDirtyFlags & TLPV_DIRTY_VB) && (triCnt > (VertCnt/4))) ||	// new VB, most verts are used
         ((!pRc->tl.dwDirtyFlags & TLPV_DIRTY_VB) &&  pRc->tl.transformedVB) )	// we've alreay seen this one and it's already transformed
    {
	  /* 
	  *	Looks like they're using almost every vert so we'll just transform them all
	  */
	  if(pRc->tl.dwDirtyFlags & TLPV_DIRTY_VB)
      {
	    pRc->tl.pTLK->K3dXformFnp ( pRc, (DWORD*)pRc->tl.InFVF.lpvData, pTLBN, VertCnt );
#ifdef DEBUG
		TnlStats_VertsXfrmd += VertCnt;
#endif
      }
      else   
        D3DPRINT( 32, "All verts already transformed");

      // For each Triangle in the list
	  while(triCnt--)
      {
	    pA = &pTLBN[(DWORD)pIndx[0]];
	    pB = &pTLBN[(DWORD)pIndx[1]];
	    pC = &pTLBN[(DWORD)pIndx[2]];

	    if (CullCheck_C(cullMask, pA, pB, pC) == 0)
		{
#ifdef DEBUG
	      if (K3D_GROUP_NOT_LIT(pA))	TnlStats_VertsXfrmdAndLit++;
	      if (K3D_GROUP_NOT_LIT(pB))	TnlStats_VertsXfrmdAndLit++;
	      if (K3D_GROUP_NOT_LIT(pC))	TnlStats_VertsXfrmdAndLit++;
#endif
	      if (K3D_GROUP_NOT_LIT(pA))
	  	    FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[0], pA );
	      if (K3D_GROUP_NOT_LIT(pB))
	  	    FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[1], pB );
	      if (K3D_GROUP_NOT_LIT(pC))
	  	    FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[2], pC );

          // Call on UN-CLIPED Rendering code to output this triangle
          TL_RENDER_Default_NoClip( &pRc->tl.KniRC, pA, pB, pC );
		}
	    pIndx += 3;
	  }
	  pRc->tl.transformedVB = TRUE;
      pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_VB;
    }
	else if ( (triCnt > (VertCnt/8)) || 					// a small portion of the verts are used
	    (!(pRc->tl.dwDirtyFlags & TLPV_DIRTY_VB)) )			// some of the verts are transformed
	{
	  /*
	  *	They're using some of the verts, we'll flag them all 
	  * as untransformed and transform only the ones we need.
	  */
	  DWORD* pSrcData = (DWORD*)pRc->tl.InFVF.lpvData;
	  DWORD dwStrideInFVF = pRc->tl.InFVF.dwStride;

	  if(pRc->tl.dwDirtyFlags & TLPV_DIRTY_VB)
	  {
	    // Dirty VB so mark every vert as untransformed
	    DWORD* pFlag = (DWORD*)&pTLBN->wfbi;
		DWORD dirtyVertCnt = VertCnt;
	    while(dirtyVertCnt--)
		{
		  *pFlag = TL_VERT_NOT_XFORMED;
		  pFlag = (DWORD*)((DWORD)pFlag + sizeof(TLBN));
		}
	  }
      else   
        D3DPRINT( 32, "Some verts already transformed but all %d flaged", VertCnt );

      // For each Triangle in the list
	  while(triCnt--)
      {
	    pA = &pTLBN[(DWORD)pIndx[0]];
	    pB = &pTLBN[(DWORD)pIndx[1]];
	    pC = &pTLBN[(DWORD)pIndx[2]];
#ifdef DEBUG
	    if(K3D_GROUP_NOT_TRANSFORMED(pA))		TnlStats_VertsXfrmd++;
	    if(K3D_GROUP_NOT_TRANSFORMED(pB))		TnlStats_VertsXfrmd++;
	    if(K3D_GROUP_NOT_TRANSFORMED(pC))		TnlStats_VertsXfrmd++;
#endif
	    if(K3D_GROUP_NOT_TRANSFORMED(pA))
	      pRc->tl.pTLK->K3dXformFnp( pRc, (DWORD*)((DWORD)pSrcData + ((DWORD)pIndx[0])*dwStrideInFVF), pA, 1 );
	    if(K3D_GROUP_NOT_TRANSFORMED(pB))
	      pRc->tl.pTLK->K3dXformFnp( pRc, (DWORD*)((DWORD)pSrcData + ((DWORD)pIndx[1])*dwStrideInFVF), pB, 1 );
	    if(K3D_GROUP_NOT_TRANSFORMED(pC))
	      pRc->tl.pTLK->K3dXformFnp( pRc, (DWORD*)((DWORD)pSrcData + ((DWORD)pIndx[2])*dwStrideInFVF), pC, 1 );

	    if (CullCheck_C(cullMask, pA, pB, pC) == 0)
		{
#ifdef DEBUG
	      if (K3D_GROUP_NOT_LIT(pA))	TnlStats_VertsXfrmdAndLit++;
	      if (K3D_GROUP_NOT_LIT(pB))	TnlStats_VertsXfrmdAndLit++;
	      if (K3D_GROUP_NOT_LIT(pC))	TnlStats_VertsXfrmdAndLit++;
#endif
	      if (K3D_GROUP_NOT_LIT(pA))
	  	    FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[0], pA );
	      if (K3D_GROUP_NOT_LIT(pB))
	  	    FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[1], pB );
	      if (K3D_GROUP_NOT_LIT(pC))
	  	    FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[2], pC );

          // Call on UN-CLIPED Rendering code to output this triangle
          TL_RENDER_Default_NoClip( &pRc->tl.KniRC, pA, pB, pC );
		}
	    pIndx += 3;
	  }
	  pRc->tl.transformedVB = FALSE;
      pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_VB;
    }
	else
	{
	  /*
	  *	Almost all of this vertex buffer won't be used.  Since flaging every 
	  * TLBN would require touching way more verts we'll bite the bullet and 
	  * transform every vert every tri.
	  */
	  DWORD* pSrcData = (DWORD*)pRc->tl.InFVF.lpvData;
	  DWORD dwStrideInFVF = pRc->tl.InFVF.dwStride;

      // For each Triangle in the list
	  while(triCnt--)
      {
	    pA = &pTLBN[(DWORD)pIndx[0]];
	    pB = &pTLBN[(DWORD)pIndx[1]];
	    pC = &pTLBN[(DWORD)pIndx[2]];
	    pRc->tl.pTLK->K3dXformFnp( pRc, (DWORD*)((DWORD)pSrcData + ((DWORD)pIndx[0])*dwStrideInFVF), pA, 1 );
	    pRc->tl.pTLK->K3dXformFnp( pRc, (DWORD*)((DWORD)pSrcData + ((DWORD)pIndx[1])*dwStrideInFVF), pB, 1 );
	    pRc->tl.pTLK->K3dXformFnp( pRc, (DWORD*)((DWORD)pSrcData + ((DWORD)pIndx[2])*dwStrideInFVF), pC, 1 );
#ifdef DEBUG
		TnlStats_VertsXfrmd += 3;
#endif

	    if (CullCheck_C(cullMask, pA, pB, pC) == 0)
		{
	  	  FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[0], pA );
	  	  FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[1], pB );
	  	  FP_XformLightK3d_Asm( pRc, (DWORD)pIndx[2], pC );
#ifdef DEBUG
	      TnlStats_VertsXfrmdAndLit += 3;
#endif

          // Call on UN-CLIPED Rendering code to output this triangle
          TL_RENDER_Default_NoClip( &pRc->tl.KniRC, pA, pB, pC );
		}
	    pIndx += 3;
	  }
	  pRc->tl.transformedVB = FALSE;
      pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VB;
    }
  } // end of non-clipped vertex loops

  TL_RENDER_Cleanup ( pRc );
  __asm  femms
  return !D3D_OK;  
}	//FP_IndexedTriangleList2_K3D()




/*-------------------------------------------------------------------
Function Name:  SetupTL_K3DTMP
Description:    Sets up some values in TL_K3DTMP
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
void SetupTL_K3DTMP(RC *pRc)
{
	TL_K3DTMP *pTlkTmp = (TL_K3DTMP *)pRc->tl.pTLK;
	DWORD	xform = 0;
	int i;

    // Setup GEOM DevCoord Info
	pRc->tl.ViewData.viewDataOffsetX = pRc->tl.ViewData.offsetX + pRc->sst.pixelOffset;
	pRc->tl.ViewData.viewDataOffsetY = pRc->tl.ViewData.offsetY + pRc->sst.pixelOffset;

	pTlkTmp->TxScale[0].u = pRc->sst.scaleS;
	pTlkTmp->TxScale[0].v = pRc->sst.scaleT;
	pTlkTmp->TxScale[1].u = pRc->sst.scaleS1;
	pTlkTmp->TxScale[1].v = pRc->sst.scaleT1;
	pTlkTmp->TxCenter[0].u = pRc->sst.centerS;
	pTlkTmp->TxCenter[0].v = pRc->sst.centerT;
	pTlkTmp->TxCenter[1].u = pRc->sst.centerS1;
	pTlkTmp->TxCenter[1].v = pRc->sst.centerT1;
	pTlkTmp->viewDataScaleZ  = pRc->zScale * pRc->tl.ViewData.scaleZ;	// pre-calculate these
	pTlkTmp->viewDataOffsetZ = pRc->zScale * pRc->tl.ViewData.offsetZ;	// (saves 1 mul per vert)

	// select special case transformation function
    if (pRc->tl.InFVF.dwFVFType & D3DFVF_DIFFUSE)	xform |= K3DXFORMBIT_DIFFUSE_COLOR;
    if (pRc->tl.InFVF.dwFVFType & D3DFVF_SPECULAR)	xform |= K3DXFORMBIT_SPECULAR_COLOR;
	for( i=0; i<TLMAX_USER_CLIPPLANES; i++)
	{
		if( pRc->tl.xfmUserClipPlanes[i].bActive )
		{
			xform |= K3DXFORMBIT_USERCLIPPLANE;
			i = TLMAX_USER_CLIPPLANES;
		}
	}
    if (pRc->tl.dwTLState & TLPV_DOCLIPPING)	xform |= K3DXFORMBIT_CLIPPED;
	xform |= pRc->tl.numVertexBlends << K3DXFORMBIT_VERTEX_BLENDS_SHIFT;
	pTlkTmp->K3dXformFnp = K3D_Xform_fns[xform];
}  

void SetupTL_SOATMP(RC *pRc)
{
  __asm 
  {
    mov     eax, pRc                // eax = pointer to pRc
    mov     ebx, [eax]RC.tl.pTL     // ebx = pointer to pRc->tl

    // Setup GEOM DevCoord Info
    movss   xmm0, [eax]RC.sst.pixelOffset       // Napalm specific pixel offset
    movss   xmm1, [eax]RC.zScale                //  and Z scale
    movss   xmm2, [eax]RC.tl.ViewData.offsetX   // Get the Offset X
    movss   xmm3, [eax]RC.tl.ViewData.offsetY   // Get the Offset Y
    shufps  xmm1, xmm1, 0                       // Broadcast the scale!
    movss   xmm4, [eax]RC.tl.ViewData.offsetZ   // Get the Offset Z
    addss   xmm2, xmm0                          // Pre-add the HW Offset
    movss   xmm5, [eax]RC.tl.ViewData.scaleX    // Get the scale X
    addss   xmm3, xmm0                          // Pre-add the HW Offset
    shufps  xmm4, xmm4, 0
    movss   xmm6, [eax]RC.tl.ViewData.scaleY    // Get the scale X
    shufps  xmm5, xmm5, 0                      
    movss   xmm7, [eax]RC.tl.ViewData.scaleZ    // Get the Scale Z
    shufps  xmm6, xmm6, 0
    movaps  [ebx]_STL.fSOAhwZScale, xmm1		// fSOAhwZScale = zScale
    shufps  xmm7, xmm7, 0   
    movaps  [ebx]_STL.fSOAviewDataScaleX, xmm5	// fSOAviewDataScaleX = ViewData.scaleX
    shufps  xmm2, xmm2, 0                       
    movaps  [ebx]_STL.fSOAviewDataScaleY, xmm6	// fSOAviewDataScaleY = ViewData.scaleY
    shufps  xmm3, xmm3, 0
    movaps  [ebx]_STL.fSOAviewDataScaleZ, xmm7	// fSOAviewDataScaleZ = ViewData.zScale
    movaps  [ebx]_STL.fSOAviewDataOffsetZ, xmm4	// fSOAviewDataOffsetZ = ViewData.offsetZ
    movaps  [ebx]_STL.fSOAviewDataOffsetX, xmm2	// fSOAviewDataOffsetX = ViewData.offsetX + sst.pixelOffset
    movaps  [ebx]_STL.fSOAviewDataOffsetY, xmm3	// fSOAviewDataOffsetX = ViewData.offsetX + sst.pixelOffset

    // Setup Texture DevCoord Info & W buffer stuff
    movlps  xmm4,[eax]RC.sst.scaleS
    movhps  xmm4,[eax]RC.sst.centerS
    movlps  xmm5,[eax]RC.sst.scaleS1
    movhps  xmm5,[eax]RC.sst.centerS1
    movss   xmm6, [eax]RC.aW
    movss   xmm7, [eax]RC.bW
    movlps  [ebx+0]_STL.fSOAscaleST0, xmm4
    movlps  [ebx+8]_STL.fSOAscaleST0, xmm4
    shufps  xmm6, xmm6, 0                       // Broadcast the scale!
    movhps  [ebx+0]_STL.fSOAcenterST0, xmm4
    movhps  [ebx+8]_STL.fSOAcenterST0, xmm4
    shufps  xmm7, xmm7, 0                       // Broadcast the offset!
    movlps  [ebx+0]_STL.fSOAscaleST1, xmm5
    movlps  [ebx+8]_STL.fSOAscaleST1, xmm5
    movhps  [ebx+0]_STL.fSOAcenterST1, xmm5
    movhps  [ebx+8]_STL.fSOAcenterST1, xmm5
    movaps  [ebx]_STL.fSOAwbuffScale, xmm6
    movaps  [ebx]_STL.fSOAwbuffOffset, xmm7
  }
}  




/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
#ifdef PROFILE_VB_ORDER

#ifdef STANDALONE
#define PRINT(x) printf(x)
#else
#define PRINT(x) D3DPRINT( 0, "%s", x)
#endif


/*-------------------------------------------------------------------
Function Name:  gather_vertex_buffer_stats
Description:    Driver for the vertex buffer orderliness profiling code
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
void gather_vertex_buffer_stats(DWORD tri_count, WORD *pIndx, DWORD vStart)
{
  static Counter = 0;

  if (profctrl == 0) {
    tri_stat.tri_list = tri_list;
    tri_stat_curr.tri_list = tri_list;
    InitIndexStats(&tri_stat);
    profctrl = 1;
  }
  else if (profctrl == 1) {
    AccumulateIndexStats(tri_count, pIndx, vStart, &tri_stat);
    InitIndexStats(&tri_stat_curr);
    AccumulateIndexStats(tri_count, pIndx, vStart, &tri_stat_curr);
    SummarizeIndexStats(&tri_stat_curr);
    // Winice can't capture everything put into the history buffer. So to keep this a little saner,
    // I'm only printing out 1 out of every 1000 sets of SOA data.

    if ((disp_flags & 1) && tri_stat_curr.ooo_flag)  {
      if (Counter++ >= 1000) {
        Display_Index_Stats_Results(&tri_stat_curr, 1);
        Counter = 0;
      }
    }
    if (disp_flags & 2) {
      if (Counter++ >= 1000) {
        Results(&tri_stat_curr, 1);
        Counter = 0;
      }

    }


  }
  else if (profctrl == 2) {
    SummarizeIndexStats(&tri_stat);
    Results(&tri_stat, 0);
    profctrl = 0;
  }
}  

/*-------------------------------------------------------------------
Function Name:  InitIndexStats
Description:    Initialize the VB order statistics data
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
void InitIndexStats(SOA_IDX_STATS *pTriStat) {
  int i;

  for (i = 0; i < NUM_BINS; i++) 
    pTriStat->back_bin[i] = 0.0f;

  pTriStat->pref_count = 0;
  pTriStat->count_prims = 0;

  pTriStat->count_soaA = 0;
  pTriStat->count_soaB = 0;
  pTriStat->count_soaC = 0;
  pTriStat->pct_soaA = 0.0f;
  pTriStat->pct_soaB = 0.0f;
  pTriStat->pct_soaC = 0.0f;

  pTriStat->min_back = 80000;
  pTriStat->max_back = 0;
  pTriStat->ooo_back = 0;
  pTriStat->count_back = 0;
  pTriStat->count_curr = 0;
  pTriStat->dist_back = 0;
  pTriStat->avg_back = 0.0f;
  pTriStat->avg_back_dist = 0.0f;

  pTriStat->pct_back4 = 0.0f;
  pTriStat->pct_back8 = 0.0f;
  pTriStat->pct_back16 = 0.0f;
  pTriStat->pct_back21 = 0.0f;
  pTriStat->pct_back32 = 0.0f;

  pTriStat->min_fwd = 80000;
  pTriStat->max_fwd = 0;
  pTriStat->ooo_fwd = 0;
  pTriStat->dist_fwd = 0;
  pTriStat->count_fwd = 0;
  pTriStat->avg_fwd = 0.0f;
  pTriStat->avg_fwd_dist = 0.0f;
}


/*-------------------------------------------------------------------
Function Name:  AccumulateIndexStats
Description:    Gather statistics about the order of indices in the vertex buffer
Parameters:   
Information:    For the purposes of this function, out of order is defined as:
                A reference backwards in the vertex buffer to an SOA group that 
                has not been previously "seen."  - or -
                A reference forwards in the vertex buffer to an SOA group that
                isn't numerically the next SOA group. 

                We also gather data on backwards references to vertices that have 
                already been seen. Note that we don't collect info on forward references 
                to vertices that we've already seen.

Return:         
-------------------------------------------------------------------*/

void AccumulateIndexStats(DWORD tri_count, WORD *pIndx, DWORD vStart, SOA_IDX_STATS *pTriStat)
{
  DWORD i; 
  DWORD  A;
  DWORD pageA, bitA;
  DWORD  B;
  DWORD pageB, bitB;
  DWORD  C;
  DWORD pageC, bitC;
  DWORD pref_count = 0;
  DWORD dist;
  DWORD currSOA = pIndx[vStart+0];
  SOA_IDX_DAT *pTriList = pTriStat->tri_list;


  // ooo_flag == 1 means that some vertices in this buffer were "out of order"
  pTriStat->ooo_flag = 0;

  if (tri_count > MAXNUMVERTICES/3)
    tri_count = MAXNUMVERTICES/3;


  // count of primitives over which this data is accumulated
  pTriStat->count_prims++;

  // reset our "seen" bitfields
  memset((void *) dwSSEIdxSeenBitfield, 0, MAX_SSE_GROUPS*sizeof(DWORD));

  // back up prior to the first SOA index group struct. This makes the loop work out OK.
  pTriList--;

  for (i = 0; i < tri_count; i++, pIndx += 3) {
    // Get the indices for the SOA groups needed for A, B, and C
    GRAB_NEXT_INDEX_VB(A, 0, pageA, bitA);
    A = SOA_IDX(A);
    GRAB_NEXT_INDEX_VB(B, 1, pageB, bitB);
    B = SOA_IDX(B);
    GRAB_NEXT_INDEX_VB(C, 2, pageC, bitC);
    C = SOA_IDX(C);

    // Have we seen this group before?
    if (!IS_SOA_GROUP_SEEN(pageA, bitA)) {
      // Nope, haven't seen it.
      pTriStat->count_soaA++;     // Tally number of times vA was unseen

      if (A < currSOA) {
        pTriStat->ooo_back++;     // Tally out of order backwards references
        pTriStat->ooo_flag = 1;   // Set out of order flag
      }
      else if (A > currSOA+4) {
        pTriStat->ooo_fwd++;      // Out of order forwards
        pTriStat->ooo_flag = 1;   // Set ooo flag
        dist = (A - currSOA);     // calc. distance
        pTriStat->count_fwd++;    // tally number of fwd references to unseen SOA groups
        pTriStat->dist_fwd += dist;
        pTriStat->min_fwd = (pTriStat->min_fwd > dist) ? dist : pTriStat->min_fwd;
        pTriStat->max_fwd = (pTriStat->max_fwd < dist) ? dist : pTriStat->max_fwd;
      }
      // Set current SOA group #
      currSOA = A;
      pTriList++;               // advance to next SOA group struct
      pTriStat->pref_count++;   // bump number of new SOA groups seen "the prefetch count" This is the count of unique SOA groups in the VB
      pTriList->new_soa = A;
      pTriList->num_tris = 0;
      SET_SOA_GROUP_SEEN(pageA, bitA);
      pTriStat->count_curr++;
      //            printf("Grab SOA %d\n", A);
    }
    else {
      // We've seen this SOA group before...
      if (A < currSOA) {
        dist = (currSOA - A);
        pTriStat->count_back++;   
        pTriStat->dist_back += dist;
        pTriStat->min_back = (pTriStat->min_back > dist) ? dist : pTriStat->min_back;
        pTriStat->max_back = (pTriStat->max_back < dist) ? dist : pTriStat->max_back;
        if ((dist / 4) < NUM_BINS)
          pTriStat->back_bin[dist/4]++;
        else
          pTriStat->back_bin[NUM_BINS-1]++;
      }
      else 
        pTriStat->count_curr++;
    }


    // Pretty much the same deal as above for Vertex B
    if (!IS_SOA_GROUP_SEEN(pageB, bitB)) {
      pTriStat->count_soaB++;
      if (B < currSOA) {
        pTriStat->ooo_back++;
        pTriStat->ooo_flag = 1;
      }
      else if (B > currSOA+4) {
        pTriStat->ooo_fwd++;
        pTriStat->ooo_flag = 1;
        dist = (B - currSOA);
        pTriStat->count_fwd++;    
        pTriStat->dist_fwd += dist;
        pTriStat->min_fwd = (pTriStat->min_fwd > dist) ? dist : pTriStat->min_fwd;
        pTriStat->max_fwd = (pTriStat->max_fwd < dist) ? dist : pTriStat->max_fwd;
      }
      currSOA = B;
      pTriList++;
      pTriStat->pref_count++;
      pTriList->new_soa = B;
      pTriList->num_tris = 0;
      SET_SOA_GROUP_SEEN(pageB, bitB);
      pTriStat->count_curr++;
      //            printf("Grab SOA %d\n", B);
    }
    else {

      if (B < currSOA) {
        dist = (currSOA - B);
        pTriStat->count_back++;   
        pTriStat->dist_back += dist;
        pTriStat->min_back = (pTriStat->min_back > dist) ? dist : pTriStat->min_back;
        pTriStat->max_back = (pTriStat->max_back < dist) ? dist : pTriStat->max_back;
        if ((dist / 4) < NUM_BINS)
          pTriStat->back_bin[dist/4]++;
        else
          pTriStat->back_bin[NUM_BINS-1]++;
      }
      else 
        pTriStat->count_curr++;
    }


    // Vertex C - this is like the other two cases, but, when we get to vertex C, we can render a triangle!
    if (!IS_SOA_GROUP_SEEN(pageC, bitC)) {
      pTriStat->count_soaC++;
      if (C < currSOA) {
        pTriStat->ooo_back++;
        pTriStat->ooo_flag = 1;
      }
      else if (C > currSOA+4) {
        pTriStat->ooo_fwd++;
        pTriStat->ooo_flag = 1;
        dist = (C - currSOA);
        pTriStat->count_fwd++;    
        pTriStat->dist_fwd += dist;
        pTriStat->min_fwd = (pTriStat->min_fwd > dist) ? dist : pTriStat->min_fwd;
        pTriStat->max_fwd = (pTriStat->max_fwd < dist) ? dist : pTriStat->max_fwd;
      }
      currSOA = C;
      pTriList++;
      pTriStat->pref_count++;
      pTriList->new_soa = C;
      pTriList->num_tris = 1;
      SET_SOA_GROUP_SEEN(pageC, bitC);
      pTriStat->count_curr++;
      //             printf("Grab SOA %d\n", C);
    }
    else {
      pTriList->num_tris++;     // bump the count of triangles that can be rendered once this SOA group has been
                                // processed.

      if (C < currSOA) {
        dist = (currSOA - C);
        pTriStat->count_back++;   
        pTriStat->dist_back += dist;
        pTriStat->min_back = (pTriStat->min_back > dist) ? dist : pTriStat->min_back;
        pTriStat->max_back = (pTriStat->max_back < dist) ? dist : pTriStat->max_back;
        if ((dist / 4) < NUM_BINS)
          pTriStat->back_bin[dist/4]++;
        else
          pTriStat->back_bin[NUM_BINS-1]++;
      }
      else 
        pTriStat->count_curr++;


    }
    //         printf("Draw Triangle\n");
  }
  //     printf("\n\n");
}    


/*-------------------------------------------------------------------
Function Name:  SummarizeIndexStats
Description:    Compute statistical informaiton about the order of vertices in an index vertex buffer
Parameters:   
Information:    
Return:         
----------------------------------------------------------------*/
void SummarizeIndexStats(SOA_IDX_STATS *pTriStat)
{
  int i;

  if (pTriStat->count_curr > 0)
    pTriStat->avg_back = (float) pTriStat->count_back / (float) pTriStat->count_curr;

  if (pTriStat->count_back > 0)
    pTriStat->avg_back_dist = (float) pTriStat->dist_back / (float) pTriStat->count_back;

  if (pTriStat->count_curr > 0)
    pTriStat->avg_fwd = (float) pTriStat->count_fwd / (float) pTriStat->pref_count;

  if (pTriStat->count_fwd > 0)
    pTriStat->avg_fwd_dist = (float) pTriStat->dist_fwd / (float) pTriStat->count_fwd;

  pTriStat->min_back /= 4;
  pTriStat->max_back /= 4;
  pTriStat->avg_back_dist /= 4.0;
  pTriStat->min_fwd /= 4;
  pTriStat->max_fwd /= 4;
  pTriStat->avg_fwd_dist /= 4.0;

  if (pTriStat->count_back) {
    for (i = 0; i < NUM_BINS; i++)
      pTriStat->back_bin[i] /= (float)pTriStat->count_back;
    for (i = 0; i < 4; i++)
      pTriStat->pct_back4 += pTriStat->back_bin[i];
    for (i = 0; i < 8; i++)
      pTriStat->pct_back8 += pTriStat->back_bin[i];
    for (i = 0; i < 16; i++)
      pTriStat->pct_back16 += pTriStat->back_bin[i];
    for (i = 0; i < 21; i++)
      pTriStat->pct_back21 += pTriStat->back_bin[i];
    for (i = 0; i < 32; i++)
      pTriStat->pct_back32 += pTriStat->back_bin[i];
  }

  pTriStat->pct_soaA = (float) pTriStat->count_soaA / (float) pTriStat->pref_count;
  pTriStat->pct_soaB = (float) pTriStat->count_soaB / (float) pTriStat->pref_count;
  pTriStat->pct_soaC = (float) pTriStat->count_soaC / (float) pTriStat->pref_count;
}


/*-------------------------------------------------------------------
Function Name:  Display_Index_Stats_Results
Description:    Display the results of vertex buffer order statistics to the debugger
Parameters:   SOA_IDX_STATS *pTriStat - VB stats to display
              int flag - 0 means don't display the SOA group data
                         1 means display the SOA group data. (this is big!)
Information:    
Return:         
-------------------------------------------------------------------*/
void Display_Index_Stats_Results(SOA_IDX_STATS *pTriStat, int flag)
{
    DWORD i; 
    DWORD sum = 0;
    DWORD order_flag = 0;
    SOA_IDX_DAT *pTriList = pTriStat->tri_list;
    char buf[160];


    sprintf(buf, "Number of primitives: %d", pTriStat->count_prims);
    PRINT(buf);
    sprintf(buf, "ooo_fwd: %d, ooo_back: %d", pTriStat->ooo_fwd, pTriStat->ooo_back);
    PRINT(buf);
    sprintf(buf, "ooo_fwd %%: %f, ooo_back: %% %f", (float)pTriStat->ooo_fwd / (float)pTriStat->pref_count , (float)pTriStat->ooo_back / (float)pTriStat->pref_count);
    PRINT(buf);
    sprintf(buf, "unique SOA groups count: %d", pTriStat->pref_count);
    PRINT(buf);
    sprintf(buf, "    count of (seen) back ref: %d, avg back: %f, min dist: %d, max dist: %d, avg dist: %f",
                pTriStat->count_back, pTriStat->avg_back, pTriStat->min_back, pTriStat->max_back, pTriStat->avg_back_dist);
    PRINT(buf);
    sprintf(buf, "    count (unseen) fwd ref: %d, avg fwd: %f, min dist: %d, max dist: %d, avg dist: %f",
                pTriStat->count_fwd, pTriStat->avg_fwd, pTriStat->min_fwd, pTriStat->max_fwd, pTriStat->avg_fwd_dist);
    PRINT(buf);

    sprintf(buf, "    %% of times a vertex was unseen by vertex -  VertA: %f, VertB: %f, VertC: %f", pTriStat->pct_soaA, pTriStat->pct_soaB, pTriStat->pct_soaC);
    PRINT(buf); 
    sprintf(buf, "    count of times a vertex was unseen by vertex -  VertA: %d, VertB: %d, VertC: %d", pTriStat->count_soaA, pTriStat->count_soaB, pTriStat->count_soaC);
    PRINT(buf); 
    
    sprintf(buf, "    %% back < 4 SOA groups = %f", pTriStat->pct_back4);
    PRINT(buf);
    sprintf(buf, "    %% back < 8 SOA groups = %f", pTriStat->pct_back8);
    PRINT(buf);
    sprintf(buf, "    %% back < 16 SOA groups = %f", pTriStat->pct_back16);
    PRINT(buf);
    sprintf(buf, "    %% back < 21 SOA groups = %f", pTriStat->pct_back21);
    PRINT(buf);
    sprintf(buf, "    %% back < 32 SOA groups = %f", pTriStat->pct_back32);
    PRINT(buf);

    if (flag) {
      sprintf(buf, "SOA group data:");
      PRINT(buf);

      sprintf(buf, "SOA #   Number of Triangles");
      PRINT(buf);
      for (i = 0; i < pTriStat->pref_count; i++) {
          sprintf(buf, "%04d     %02d", pTriStat->tri_list[i].new_soa/4, pTriStat->tri_list[i].num_tris);
          PRINT(buf);
      }
    }

    sprintf(buf, "\n\n");
    PRINT(buf);
    
}    

#endif // PROFILE_VB_ORDER




#endif //VCPP
#endif //TnL_HAL
#endif //DX7
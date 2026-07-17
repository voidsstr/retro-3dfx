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
** File name: tlglobal.h
**
** Description: Global defines for Software T&L HAL Code
**
** $Revision: 86$
** $Date: 10/19/00 1:30:47 PM$
**
** $Log: 
**  86   3dfx      1.54.1.7.1.2210/19/00 Allen Hansen    Added check for change in
**       VB start in indexed tris, caused failure in Evolva
**  85   3dfx      1.54.1.7.1.2110/18/00 Allen Hansen    Changed SSE T&L tag scheme
**       from a bitfield to marking the vert T&L'd in one of the textures
**  84   3dfx      1.54.1.7.1.2010/11/00 Brent           Forced check in to enforce
**       branching.
**  83   3dfx      1.54.1.7.1.1910/10/00 Allen Hansen    added vertex blends to the
**       optimized paths
**  82   3dfx      1.54.1.7.1.1810/03/00 Allen Hansen    changed TLCLIPVTX.wfbi to
**       .vfog and .color to .diffuse, removed .clip_color and .clip_specular from
**       TLCLIPPING
**  81   3dfx      1.54.1.7.1.1709/29/00 Johnny Trainor  Modification to allow the
**       DX8 driver to build with build 0181 of the DX8DDK.
**  80   3dfx      1.54.1.7.1.1609/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  79   3dfx      1.54.1.7.1.1509/22/00 Allen Hansen    implemented texgen and
**       texture transformation to the fastpaths
**  78   3dfx      1.54.1.7.1.1409/15/00 Allen Hansen    Got rid of the TLBN_CLIP
**       structure (with homogenous coords), all output verts, clipped or not, use
**       the TLBN structure.
**  77   3dfx      1.54.1.7.1.1309/13/00 Allen Hansen    Fixed bug when using
**       colorvertex; if diffuse comes from the vertex, then the alpha must come
**       from that source.  Was failing the D3DIM sample app "Shadow Volume 2"
**  76   3dfx      1.54.1.7.1.1209/05/00 Allen Hansen    Fixed bug in the swizzle
**       code: we were always swizzling blocks of 4 verts.  If the # of verts
**       wasn't divisible by 4, we always swizzled the last group of 4.  We can
**       page fault (or GPF) if the end of a VB is near the end of a page (or
**       segment). 
**  75   3dfx      1.54.1.7.1.1109/03/00 Allen Hansen    tlglobal.h, xform.c,
**       procprim.c
**  74   3dfx      1.54.1.7.1.1008/31/00 Allen Hansen    added SSE2 support,
**       cleaned out some dead code
**  73   3dfx      1.54.1.7.1.908/29/00 Allen Hansen    added colorvertex support
**       for sse path
**  72   3dfx      1.54.1.7.1.808/27/00 Allen Hansen    changed TLMAXNUMVERTICES to
**       D3DMAXNUMVERTICES+1 (was 8K), added colorvertex immediate values to
**       TLLIGHT, moved P8RGBA to soa.h, added TLPV_DIRTY_DYNGB to TLPV_DIRTY_XFORM
**  71   3dfx      1.54.1.7.1.708/23/00 Allen Hansen    Color vertex stuff
**  70   3dfx      1.54.1.7.1.608/21/00 Allen Hansen    changed from sse intrinsics
**       to inline assembly, removed BIT_RC_SPEC_AND_TEXTURES from RC bits (all
**       spec tris' are now 2 pass), moved some def's to d7d3d.h, start on
**       fast-path color vertex support, removed material from TLLIGHTING (it's
**       also in TL), added TLPV_DIRTY_EYEXFM bits, added T&L renderstate support,
**       added support for up to 16 active lights
**  69   3dfx      1.54.1.7.1.507/29/00 Allen Hansen    added pSynthIndices to
**       pRc.tl for making indexed tris out of other primitive types
**  68   3dfx      1.54.1.7.1.407/24/00 Allen Hansen    whql bug fixes (T&L tests)
**  67   3dfx      1.54.1.7.1.307/21/00 Allen Hansen    added
**       p8AmbEmissPlusSpecAlpha for K3D lighting
**  66   3dfx      1.54.1.7.1.207/03/00 Allen Hansen    3DNow stuff
**  65   3dfx      1.54.1.7.1.106/29/00 Allen Hansen    added K3dPowerFunction
**       support
**  64   3dfx      1.54.1.7.1.006/25/00 Allen Hansen    added support for 3dnow
**       functions and lighting variables
**  63   3dfx      1.54.1.7    06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  62   3dfx      1.54.1.6    05/30/00 Allen Hansen    added
**       XformDeswiz4_DevCoord_SOA_GEOM and dwTLCodePath to the RC, made the
**       rendering code compilable only with SSECPP
**  61   3dfx      1.54.1.5    05/22/00 Allen Hansen    Added function definitions
**       used by 2-pass outer loop
**  60   3dfx      1.54.1.4    05/11/00 Scott Kephart   More optimizations for
**       lighting
**  59   3dfx      1.54.1.3    05/11/00 Allen Hansen    added
**       Xform_DevCoord_SOA_GEOM_Funct to the TL_RENDERCONTEXT_DATA
**  58   3dfx      1.54.1.2    05/10/00 Allen Hansen    Wrote KNI versions of
**       Inverse4x4, MatrixProduct, and TransposeMatrix functions
**  57   3dfx      1.54.1.1    05/05/00 Bob Johnston    Implimented Dynamic
**       Guardband clipper for SW T&L HAL
**  56   3dfx      1.54.1.0    05/03/00 Allen Hansen    Moved deswizzling of color,
**       fog, and textures into the texture transform functions, which eliminates
**       storing and loading these params
**  55   Napalm Shared1.54        04/26/00 Bob Johnston    use of local function
**       pointers to help eliminate branching in the vertex loops.  Setup a Fn
**       pointer for Xform_DevCoord_SOA_GEOM
**  54   Napalm Shared1.53        04/26/00 Bob Johnston    Matching Matt's Changes
**       to the HW GuardBand Clip
**       -200,-200, 2048, 1536
**  53   Napalm Shared1.52        04/25/00 Allen Hansen    got rid of old
**       XForm_3/4_SOA functions (were unused)
**  52   Napalm Shared1.51        04/19/00 Scott Kephart   Big lighting change -
**       Part I
**       Lighting is now split into two parts, diffuse and specular. 
**  51   Napalm Shared1.50        04/17/00 Bob Johnston    Optimized the VB Split
**       vertex loop to use a byte array.  Shows a small improvement perfrormance.
**  50   Napalm Shared1.49        04/12/00 Allen Hansen    1) changed input param
**       to TNL rendering function from *RC to *TL_RENDER_DATA
**       2) added prefetching to single-swizzle code, requires new file
**       asoaswiz.asm
**  49   Napalm Shared1.48        04/10/00 Allen Hansen    Worked prefetching:
**       Changed FP_IndexedTriangleList2_SOA_Split() to transform 4 soa groups
**       instead of 1.  This required modifing AllocateSOAFVF().  I added a version
**       of FP_Xform_4Vert_SOA() that prefetches the next SOA group.  This still
**       needs optimization, but almost all cache-read misses are hidden.
**  48   Napalm Shared1.47        04/09/00 Allen Hansen    Prefetch work to all but
**       position x/y, some optimization work to the 3x3 and 4x3 transforms
**  47   Napalm Shared1.46        03/29/00 Bob Johnston    Created User Memory
**       Split T&L Path and added index pre calculations.
**  46   Napalm Shared1.45        03/28/00 Allen Hansen    Special cased the KNI
**       T&L rendering code
**  45   Napalm Shared1.44        03/23/00 Allen Hansen    Functional assembly
**       version of the SW T&L rendering code
**  44   Napalm Shared1.43        03/23/00 Bob Johnston    Scott and Bob's changes
**       to split up the tranformation and lighting in the vertex processing loop
**       for improved VB primitive perfromance.
**  43   Napalm Shared1.42        03/21/00 Allen Hansen    Added support for
**       atlrend.asm
**  42   Napalm Shared1.41        03/17/00 Allen Hansen    SW T&L changes only...
**       Modified makefile to use MS compiler instead of Intel compiler for
**       tlrender.c
**       Did several speed modifications to tlrender.c
**       Requires a clean build (because the rendering context structure changed)
**  41   Napalm Shared1.40        03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  40   Napalm Shared1.39        03/16/00 Bob Johnston    Got Vertex Buffers
**       working correctly
**  39   Napalm Shared1.38        03/14/00 Matt McClure    Removed Wfbi, no need
**       for it now
**  38   Napalm Shared1.37        03/14/00 Scott Kephart   Added support for
**       Xform_DevColor_SOA
**  37   Napalm Shared1.36        03/08/00 Scott Kephart   Re-shuffle of T&L code.
**  36   Napalm Shared1.35        03/07/00 Scott Kephart   Backed out Allen's last
**       changes. 
**  35   Napalm Shared1.34        03/07/00 Allen Hansen    Some optimization work,
**       changed specular pass code to do writes to cmd fifo and spec buffer at the
**       same time
**  34   Napalm Shared1.33        03/04/00 Bob Johnston    Increased max vertices
**       for VB and User Mem to 8K.  This fixes 3DMark2K's problems.  Also but a
**       fail over check to punt to the slow path  when the driver sees more than
**       8K of vertices inthe T&L HAL.  This number can be easily adjusted
**  33   Napalm Shared1.32        03/03/00 Scott Kephart   SW T&L: re-wrote
**       DeSwizzleSOAToTLBN This allows  us to change the ordering of members in
**       the TLBN struct without breaking code. 
**       Added 1 dword of padding to TLBN. Moved clip_code to before the texture
**       coordinates in TLBN_CLIP. (Same position as pad DWORD in TLBN).
**  32   Napalm Shared1.31        03/01/00 Matt McClure    Modified TLCLIPVTX
**       structure to include WFBI
**  31   Napalm Shared1.30        02/28/00 Bob Johnston    Optimization to carry
**       XMM register values between blocks of code.  Turned on bt the
**       CARRY_XMM_REG_VALUES define in tlglobal.h
**  30   Napalm Shared1.29        02/25/00 Allen Hansen    tlrend.c: fixed flat
**       shading
**       tlkinmath.asm & tlglobal.h: added KniExp function, will be used by fog
**  29   Napalm Shared1.28        02/25/00 Allen Hansen    fixed specular
**       autostripping bug (required another param in the TL_RENDER_DATA
**       structure), 
**       optimized the autostripping a bit, 
**       fixed a couple of compiler warnings (signed/unsigned mismatch w/WRAP,
**       unused var's in the clip code,
**       deleted several unneeded TL_RENDER_DATA updates in the clip code when
**       forcing the next tri to full
**  28   Napalm Shared1.27        02/23/00 Scott Kephart   Specular now works for
**       directional and point lights
**  27   Napalm Shared1.26        02/23/00 Bob Johnston    Fastpath clipping with
**       auto stripping, trivial accept, reject and better configuration.
**  26   Napalm Shared1.25        02/22/00 Matt McClure    Added function
**       declaration for TL_RENDER_DEFAULT_CLIP
**  25   Napalm Shared1.24        02/22/00 Bob Johnston    Fastpath rendering with
**       non-clipped vertices.  Texture coordinate handleing bug fixes, lots of bug
**       fixes.  Still showing some bugs in the fastpath.
**  24   Napalm Shared1.23        02/21/00 Bob Johnston    Optimized Viewport
**       transform's use of texture coords and made changes in the swizzle of Text
**       coords into SOAFVF.  Also added support in the vertex proceesing loop to
**       call on the newly added fast path rendering code.  Added necessary changes
**       to the T&L context data structure to support rendering
**  23   Napalm Shared1.22        02/16/00 Bob Johnston    Completed full viewport
**       transformation code to spec.  Updated test code to use new viewport xform.
**  22   Napalm Shared1.21        02/15/00 Scott Kephart   One more fix from
**       previous bad check-in
**  21   Napalm Shared1.20        02/15/00 Scott Kephart   Oops -- somehow the last
**       version got trashed in my editor during checkin.
**  20   Napalm Shared1.19        02/15/00 Scott Kephart   Updates for lighting
**  19   Napalm Shared1.18        02/14/00 Scott Kephart   re-enable fastpath
**  18   Napalm Shared1.17        02/14/00 Scott Kephart   Cleanup of SOA light
**       structures
**  17   Napalm Shared1.16        02/14/00 Scott Kephart   Changes for fastpath
**       lighting. Totally restructured the TLLIGHT structure so that the slowpath
**       and fastpath could share the same structure. The slowpath uses the zeroth
**       element in the SOA array.
**  16   Napalm Shared1.15        02/10/00 Scott Kephart   Data structure cleanup
**       for SOA.H -- we're unionized now!
**  15   Napalm Shared1.14        02/08/00 Scott Kephart   Added SOA_ARGB_F2I
**  14   Napalm Shared1.13        02/07/00 Scott Kephart   Updated prototype for
**       ComputeClipCodeSOA
**  13   Napalm Shared1.12        02/04/00 Bob Johnston    Changes to support User
**       Memory vertex buffers for the fastpath.  Also cleaned up the vertex
**       processing loops with neater macros for better readablity.
**  12   Napalm Shared1.11        02/03/00 Scott Kephart   added #define
**       TLPV_USERCLIPPLANES
**  11   Napalm Shared1.10        02/01/00 Scott Kephart   More lighting changes.
**       Better SSE matrix multiply code.
**  10   Napalm Shared1.9         01/28/00 Scott Kephart   Big T&L Merge: Changes
**       for fast path
**  9    Napalm Shared1.8         01/10/00 Scott Kephart   Updates to lighting.
**       Split point and spot light functions. Only allow 8 active lights to reduce
**       branch mispredictions. Cleanup in lighting.
**  8    Napalm Shared1.7         12/13/99 Scott Kephart   Big T&L Update:
**       1. Improved T&L profiling code
**       2. Optimizations to scalar transformation and lighting code
**       3. Changes to the vertex buffer code to allow functionality under Windows
**       2000.
**  7    Napalm Shared1.6         11/22/99 Scott Kephart   More vertex buffer
**       support changes. Fix for hang in 3D Mark 2000.
**  6    Napalm Shared1.5         11/11/99 Scott Kephart   More profiling code for
**       lights
**  5    Napalm Shared1.4         11/10/99 Scott Kephart   Added profiling changes
**       for T&L
**  4    Napalm Shared1.3         11/10/99 Scott Kephart   Fixed comments / added
**       #define to make setting TL specular state simpler
**  3    Napalm Shared1.2         11/03/99 Scott Kephart   add void to declaration
**       of TLHAL_DestroyRC
**  2    Napalm Shared1.1         10/26/99 Scott Kephart   rename clip.h to
**       tlclip.h to avoid a conflict with the  Windows 2K driver
**  1    Napalm Shared1.0         10/25/99 Scott Kephart   
** $
** 
** 7     3/12/00 5:19p Skephart
** Changed FogVertexSOA to pass 1 less parameter
** 
** 6     3/10/00 4:44p Skephart
** Add bits for dwTLDevFlags
** 
** 5     3/08/00 9:24p Skephart
 * 
 * 12    1/27/00 2:45a Skephart
 * 
 * 11    1/27/00 12:43a Skephart
 * New changes from Bob, Joe, Scott
 * 
 * 10    1/25/00 10:17p Skephart
 * BobJ Merge
 * 
 * 8   1/24/00 10:40p Skephart
 * Skeleton for SOA T&L
 * 
 * 7   1/24/00 4:13p Skephart
 * Beginnings of the fast path
 * 
 * 6   1/20/00 2:25p Skephart
 * Update from BobJ
 * 
 * 5   1/19/00 9:50p Skephart
 * Add and Init SOA lighting function stubs. Allocate TL_SOATMP structure
 * 
 * 4   1/19/00 2:24p Skephart
 * Bob's FVF_COMP changes part II
** 
** 2   1/17/00 9:55p Skephart
** Bob J's FVF_COMP changes
** 
** 28  10/26/99 12:52a Skephart
** Added #ifdef TnL_HAL
** 
** 27  10/25/99 11:50p Skephart
** Merge in GB Clip changes from BobJ
** 
** 26  10/20/99 3:15p Skephart
** 
** 25  10/20/99 2:45p Skephart
** 
** 24  10/20/99 2:00a Skephart
** Added new flags to tl.dwTLState & dwDirtyFlags
** 
** 23  10/07/99 4:50p Skephart
** Merged in more clip changes from Bob
** 
** 22  10/06/99 11:47p Skephart
** Merged in Bob's clipping framework
** 
** 21  10/06/99 10:27p Skephart
** Cleanup
** 
** 20  10/06/99 3:20p Skephart
** Cleanup
** 
** 19  10/01/99 4:26p Skephart
** Fog Table fixes
** 
** 18  9/28/99 3:13p Skephart
** Fixes from BobJ
** 
** 17  9/27/99 8:32p Skephart
** Move render context variables for TL HAL out of d3global.h, 
** and into tlglobal.h. All TL related variables are gathered under 
** pRc->tl.<variable>
** 
** 16  9/27/99 11:07a Skephart
** Fixed D3DDP2OP_SETLIGHT so that it doesn't corrupt the command stream.
** 
** 15  9/16/99 11:00a Skephart
** Added TLHAL_InitializeNewRC
** TLHAL_DestroyRC
*/

#ifdef TnL_HAL
#ifndef __TLGLOBAL_H__
#define __TLGLOBAL_H__

#define FASTPATH    //Undefine this to disable the fastpath code


// Intel KNI intrinsics
//#include "xmmintrin.h"  // intrinsics file

#include "soa.h"
#include "tlclip.h"
#include "tlutil.h"
#include "soaconst.h"
#include "soaldef.h"
#include "k3ddef.h"

// Default color values that should be used when ther is no lighting and
// color in vertices provided
#define TL_DEFAULT_DIFFUSE  0xFFFFFFFF
#define TL_DEFAULT_SPECULAR 0x00000000

//#define REF_GB_LEFT   -32768.f
//#define REF_GB_TOP    -32768.f
//#define REF_GB_RIGHT   32767.f
//#define REF_GB_BOTTOM  32767.f

// BobJ -- Napalm has a ±4K guardband
//#define H5_GB_LEFT   -4095.f
//#define H5_GB_TOP    -4095.f
//#define H5_GB_RIGHT   4095.f
//#define H5_GB_BOTTOM  4095.f

// Per Matt M's change in Napalm GBG parameters
#define H5_GB_LEFT    -200.f
#define H5_GB_TOP     -200.f
#define H5_GB_RIGHT   2048.f
#define H5_GB_BOTTOM  1536.f


//-----------------------------------------------------------------------------
//
// Typedefs
//
//-----------------------------------------------------------------------------

#ifdef VCPP
typedef __m128 (*KniPowerFunctionType)();
#else
typedef void (* KniPowerFunctionType)();
#endif
typedef void (* K3dPowerFunctionType)();


// Simple structure for our private use of 
// determining clip values for each vertex
// It's 16-byte aligned when compiled 
// by the Intel compiler.
typedef struct _PRVCLIPPLANES
{
  FLOAT x;
  FLOAT y;
  FLOAT z;
  FLOAT w;
} PRVCLIPPLANES, *LPPRVCLIPPLANES;

//--------------------------
//  Rendering Stuff
//--------------------------
typedef struct _TextureType {
    float   s, t;
} TextureType, *lpTextureType;

// Typedef for Fastpath rendering setup function
typedef void TL_RENDER_RenderTri_Type_NoClip ();    // this is a "false" typedef!!!

typedef struct _TL_RENDER_DATA {
    DWORD           CmdStartDiff;           // full non-spec tri start cmd
    DWORD           CmdStartSpec;           // full specular tri start cmd
    DWORD           CmdContDiff;            // part non-spec tri start cmd
    DWORD           CmdContSpec;            // part specular tri start cmd
    lpTLBN          prevC, prevB, prevA;    // previous vertex pointers (DON'T CHANGE THE ORDER!!!)
    DWORD           lastSpecRendered;       // last tri was rendered to specular buffer
    DWORD           lastTriType;            // last tri was a fan or strip (for autostripping)
    DWORD           *lpCmdFifoPtr;          // current command fifo pointer
    DWORD           dwCmdFifoRoom;          // space left in the command fifo (in bytes, not in entries)
    DWORD           dwCmdFifoBump;          // for the P6 write-buffer bug (in bytes, not entries)
    DWORD           *lpSpecBuffBase;        // array where we store specular buffer data
    DWORD           *lpSpecBuff;            // pointer to last location written to in the specular buffer
    DWORD           *SpecBufferThreshold;   // point at which to dump specular buffer
    DWORD           RfBits;                 // rendering flags
    DWORD           LastRfBits;             // RfBits from the last primitive
    TL_RENDER_RenderTri_Type_NoClip* RenderTriFunct_NoClip; // pointer to the current render function
    RC*             pRc;                    // rendering context
} TL_RENDER_DATA, *lpTL_RENDER_DATA;

typedef void TL_RENDER_RenderTri_Type_NoClip ( TL_RENDER_DATA* KniRC, lpTLBN pA, lpTLBN pB, lpTLBN pC );

extern  SOA_FLOAT   asmKniTxScaleOne;   // wrap ? scale*1.0 : 0
extern  SOA_FLOAT   asmKniTxScaleHalf;  // wrap ? scale*1.0 : 0

// The order of these MUST match the respective ones in aglobal.inc
#define BIT_RC_SPECULAR             (1 <<  0)   // pRc->specular
#define BIT_RC_FLAT_SHADING         (1 <<  1)   // pRc->shadeMode == D3DSHADE_FLAT
#define BIT_RC_REQUIRES_WFBI        (1 <<  2)   // pRc->state & SST_SETUP_Wfbi
#define BIT_RC_ALPHA                (1 <<  3)   // pRc->alphaBlendEnable
#define BIT_RC_REQUIRES_TX0         (1 <<  4)   // pRc->state & STATE_REQUIRES_ST_TMU0
#define BIT_RC_REQUIRES_TX1         (1 <<  5)   // pRc->state & STATE_REQUIRES_ST_TMU1
#define BIT_RC_WRAP_VALID           (1 <<  6)   // any wrap on a valid texture
#define BIT_RC_REQUIRES_Z           (1 <<  7)   // pRc->state & SST_SETUP_Z

#define BIT_RC_COLOR                (1 <<  8)   // pRc->sst.sSetupMode & (SST_SETUP_RGB | SST_SETUP_A)
#define BIT_RC_REQUIRES_WBUFFER     (1 <<  9)   // pRc->state & STATE_REQUIRES_WBUFFER
//#define BIT_RC_SPEC_AND_TEXTURES    (1 << 10)   // 2nd pass required for specular (specular on textures)
#define BIT_RC_VERTEX_FOG           (1 << 11)   // pRc->state & STATE_REQUIRES_VERTEXFOG
#define BIT_RC_WRAP_TX0_S           (1 << 12)   // pRc->wrapT0 & D3DWRAP_U
#define BIT_RC_WRAP_TX0_T           (1 << 13)   // pRc->wrapT0 & D3DWRAP_V
#define BIT_RC_WRAP_TX1_S           (1 << 14)   // pRc->wrapT1 & D3DWRAP_U
#define BIT_RC_WRAP_TX1_T           (1 << 15)   // pRc->wrapT1 & D3DWRAP_V
#define BIT_RC_REQUIRES_W           (1 << 16)   // pRc->state & STATE_REQUIRES_W
#define BIT_RC_REQUIRES_PERSPECTIVE (1 << 17)   // pRc->state & STATE_REQUIRES_PERSPECTIVE
#define BIT_RC_IS_NAPALM            (1 << 18)   // Napalm
#define BIT_RC_NEVER_AUTOSTRIP      (1 << 19)   // flat shaded triangles, or use for debugging autostripping
#define BIT_RC_BAD_FVF_TYPE         (1 << 20)   // force to C renderer
#define BIT_RC_FOG_ENABLE           (1 << 21)   // pRc->fogEnable
#define BIT_RC_Z_ENABLE             (1 << 22)   // pRc->zEnable
#define BIT_RC_Z_WRITE_ENABLE       (1 << 23)   // pRc->zWriteEnable

#define BITS_RC_RFUNCT_MASK         (BIT_RC_SPECULAR | BIT_RC_FLAT_SHADING | BIT_RC_REQUIRES_WFBI | BIT_RC_ALPHA | \
                                     BIT_RC_REQUIRES_TX0 | BIT_RC_REQUIRES_TX1 | BIT_RC_WRAP_VALID | BIT_RC_REQUIRES_Z)

// KNI SOA Swizzle functions in assembly (in asoaswiz.asm)
typedef void KniSwizSingle_CopyTxtrs_FunctionType();
typedef void KniSwizSingle_SwizParms_FunctionType( DWORD *lpSrc, DWORD* lpDst, DWORD dwStride, KniSwizSingle_CopyTxtrs_FunctionType* KniCpyTxtrFunctPtr );

// Special case KNI Transform Code
typedef void Xform_DevCoord_Type_SOA_GEOM ( RC *pRc );
typedef void XformDeswiz4_DevCoord_SOA_GEOM (RC *pRc, DWORD *pSOA, DWORD *pDstVert);
typedef void ConvertPackedSOAColortoSOA_RGB_type( SOA_DWORD *src, SOA_RGB *dst);


//--------------------------
//  Vertex Buffer Support
//  for managinging our own 
//  Vertex Buffers
//--------------------------

// For building with DX8DDK ("D3DMAXNUMVERTICES" is not defined in DX8DDK)

// The maximum number of vertices user can pass to any d3d
// drawing function or to create vertex buffer with
#ifndef D3DMAXNUMVERTICES
#define D3DMAXNUMVERTICES    ((1<<16) - 1)
#endif // D3DMAXNUMVERTICES


#define TLMAXNUMVERTICES  (D3DMAXNUMVERTICES+1)
#define TLMAX_SSE_GROUPS  (TLMAXNUMVERTICES>>7)
#define TLMAX_SSE_GROUPS_NEW  (TLMAXNUMVERTICES>>4)

// 96 bytes for a FULL on TLVERTEX with 8 textures + 2 reserved
// = x,y,z,w,  fog,clipcode,diff,spec,  8 textures
#define MAX_TLBUFF_VERT_SIZE    96    



// dwSrcFlags
#define VBSURF_ACCESSED           0x00000001l
#define VBSURF_WRITEONLY          0x00000002l
#define VBSURF_NOTQUALIFY         0x00000004l
#define VBSURF_JUSTCREATED        0x00000008l
#define VBSURF_VALIDCHECKSUM      0x00000010l

typedef struct _VBSURFACEDATA
{
   // Info for the Real Source 
   // Vertex Buffer.  This is
   // the memory that gets reported 
   // to the DD Surface for the 
   // Application to use directly.
   LPVOID   pSrcAllocAddr;    // Allocted Address for the Source Vertex Buffer
   LPVOID   pSrcAlignAddr;    // 32 byte Aligned Address for the Source Vertex Buffer
   DWORD    dwSrcAllocSize;   // Total memory (in bytes) allocated for Source VB
   DWORD    dwSrcAlignSize;   // Total memory (in bytes) for the aligned section of Source VB
   DWORD    dwSrcChecksum;    // DWORD Checksum of the Aligned Source Vertex Buffer
   DWORD    dwUnlockCounter;  // VB unlock history counter
   DWORD    dwSrcFlags;       // VB Source Surface flags we need to know!

   // Info for the Optimized VB
   LPVOID   pOptAllocAddr;    // Allocted Address for the Optimized Vertex Buffer
   LPVOID   pOptAlignAddr;    // 32 byte Aligned Address for the Optimized Vertex Buffer 
   DWORD    dwOptAllocSize;   // Total memory (in bytes) allocated for Optimized VB
   DWORD    dwOptAlignSize;   // Total memory (in bytes) for the aligned section of the Optimized VB
   DWORD    dwNumVecs;        // Number of gouped SOA vectors in Optimized Vertex Buffer (1 vec = 16 verts)
   DWORD    dwNumVerts;       // Number of vertices in Optimized Vertex Buffer
} VBSURFACEDATA, *LPVBSURFACEDATA;

typedef struct 
{
  D3DVALUE r,g,b;
} TLCOLOR;

//-----------------------------------------------------------------------------
// Function pointer to the lighting function
//-----------------------------------------------------------------------------
typedef void (*TLLIGHTFN)(RC *pRc);

//-----------------------------------------------------------------------------
// Function pointer to the functions that light a vertex
//-----------------------------------------------------------------------------
typedef void (*TLLIGHTVERTEXFN)( );

//-----------------------------------------------------------------------------
// Functions to compute lighting
//-----------------------------------------------------------------------------
typedef struct _TLLIGHTVERTEX_FUNC_TABLE
{
  TLLIGHTVERTEXFN pfnDirectional;
  TLLIGHTVERTEXFN pfnSpot;
  TLLIGHTVERTEXFN pfnPoint;
  TLLIGHTVERTEXFN pfnDirectionalK3d;
  TLLIGHTVERTEXFN pfnSpotK3d;
  TLLIGHTVERTEXFN pfnPointK3d;
} TLLIGHTVERTEX_FUNC_TABLE;


// Special case inline SOA lighting functions (in soalitfn.c)
typedef struct _TLLIGHT_SOA_FUNC_TABLE
{
  TLLIGHTVERTEXFN pfnDirectionalSOA_D[MAX_DIRECTIONAL_DIFF+1];
  TLLIGHTVERTEXFN pfnDirectionalSOA_S[MAX_DIRECTIONAL_SPEC+1];
  TLLIGHTVERTEXFN pfnPointSOA_D[MAX_POINT_SPOT_DIFF+1];
  TLLIGHTVERTEXFN pfnPointSOA_S[MAX_POINT_SPOT_SPEC+1];
  TLLIGHTVERTEXFN pfnSpotSOA_D[MAX_POINT_SPOT_DIFF+1];
  TLLIGHTVERTEXFN pfnSpotSOA_S[MAX_POINT_SPOT_SPEC+1];
} TLLIGHT_SOA_FUNC_TABLE; 

// Special case inline 3DNow lighting functions (in k3dlitfn.c)
extern const TLLIGHTVERTEXFN	K3D_DirectionalLight_fns[];
extern const TLLIGHTVERTEXFN	K3D_PointLight_fns[];
extern const TLLIGHTVERTEXFN	K3D_SpotLight_fns[];
extern const TLLIGHTFN pK3dLight[TLMAX_ACTIVE_LIGHTS+1];

// Special case inline 3DNow transformation functions (in k3dtnl.c)
extern const XFORMFNP	K3D_Xform_fns[];


//-----------------------------------------------------------------------------
//
// TLLight - The light struct used by the  T&L implementation
//
//-----------------------------------------------------------------------------
#define TLLIGHT_ENABLED             0x00000001  // Is the light active
#define TLLIGHT_NEEDSPROCESSING     0x00000002  // Is the light data processed
#define TLLIGHT_READY               0x00000004  // Light is ready to use

typedef struct _TLLight {
  // SOA Data
  SOA_XYZ         SOAposition_in_eye;    // In the eye space
  SOA_XYZ         SOAdirection_in_eye;   // In the eye space
  //
  // Precomputed colors scaled from 0 - 255,
  //
  SOA_RGBA        SOAMa_La;              // Material ambient times light ambient
  SOA_RGBA        SOAMd_Ld;              // Material diffuse times light diffuse
  SOA_RGBA        SOAMs_Ls;              // Material specular times light specular

  // Precomputed colors for color vertex
  SOA_RGB		  SOALa;
  SOA_RGB		  SOALd;
  SOA_RGB		  SOALs;

  SOA_FLOAT       SOAAttenuation0;       // Constant attenuation 
  SOA_FLOAT       SOAAttenuation1;       // Linear attenuation 
  SOA_FLOAT       SOAAttenuation2;       // Quadratic attenuation 

  // Stuff for SpotLights
  SOA_FLOAT       SOArange_squared;
  SOA_FLOAT       SOAcos_theta_by_2;
  SOA_FLOAT       SOAcos_phi_by_2;
  SOA_FLOAT       SOAinv_theta_minus_phi;
  SOA_FLOAT       SOASpecThresh;

  // Scratchpad area
  SOA_DWORD       SOAwritemask;          // writemask for lighting
//  SOA_FLOAT       SOAdot;                // dot product							UNUSED
  SOA_FLOAT       SOAatt;                // attenuation factor
  SOA_FLOAT       SOAdist;               // distance
//  SOA_XYZ         SOAhalfway;            // halfway vector						UNUSED
//  SOA_XYZ         SOAeye;                // incident vector -- vector from eye	UNUSED
  SOA_XYZ         SOAd;                  // direction vector from vertex to light
//  SOA_DWORD       SOAcmpres;

  // Scaler version of the same stuff
  // DO NOT RE-ALIGN THESE!!!  Need to keep these 8-byte aligned!
  P8RGBA		  p8La_U12pt0;			// ambient * 255 * 16
  P8RGBA		  p8Ld_U12pt0;			// diffuse * 255 * 16
  P8RGBA		  p8Ls_U12pt0;			// specular * 255 * 16

  // packed words with the same scaled to S12.4
  P8RGBA          p8Ma_La_U16pt0;            // Material ambient times light ambient, unsigned 16.0
  P8RGBA          p8Ma_La_U12pt4;            // Material ambient times light ambient, unsigned 12.4
  P8RGBA          p8Md_Ld_U12pt4;            // Material diffuse times light diffuse, unsigned 12.4
  P8RGBA          p8Ms_Ls_U12pt4;            // Material specular times light specular, unsigned 12.4

  D3DVECTOR       direction_in_eye;		// In the eye space
  float           range_squared;

  D3DVECTOR       position_in_eye;		// In the eye space
  float           Attenuation0;			// Constant attenuation 

  float           Attenuation1;			// Linear attenuation 
  float           Attenuation2;			// Quadratic attenuation 

  //--------- re-align anything you want after here -------
  float           cos_phi_by_2;
  float           cos_theta_by_2;
  float           inv_theta_minus_phi;


  // Precomputed colors scaled from 0 - 255,
  D3DCOLORVALUE   Ma_La;              // Material ambient times light ambient
  D3DCOLORVALUE   Md_Ld;              // Material diffuse times light diffuse
  D3DCOLORVALUE   Ms_Ls;              // Material specular times light specular

  // The rest of this is shared by soa and non-soa code
  // Scalar Data
  D3DLIGHTTYPE    dltType;               // Type of light source 
  D3DCOLORVALUE   La;                    // light ambient
  D3DCOLORVALUE   Ld;                    // light diffuse
  D3DCOLORVALUE   Ls;                    // light specular
  D3DVECTOR       Position;              // Position in world space
  D3DVECTOR       Direction;             // Direction in world space 
  D3DVALUE        Range;                 // Cutoff range 
  D3DVALUE        Falloff;               // Falloff 
  D3DVALUE        Theta;                 // Inner angle of spotlight cone 
  D3DVALUE        Phi;                   // Outer angle of spotlight cone   
    
  // Flags
  DWORD           dwFlags;

  // DotProduct sign bits for the current vertex
  DWORD           dwNonZeroDot;

  // Active List next element
  struct _TLLight *Next;

  // Specific function to light the vertex
  TLLIGHTVERTEXFN pfnLightVertex;
  // Specific SOA function to light the vertex
  TLLIGHTVERTEXFN pfnLightVertexSOA;
  TLLIGHTVERTEXFN pfnLightVertexSOA_Diff;
  TLLIGHTVERTEXFN pfnLightVertexSOA_Spec;
  // Specific function to light the vertex
  TLLIGHTVERTEXFN pfnLightVertexK3D;

} TLLIGHT, *LPTLLIGHT;

typedef TLLIGHT _LIT;

// We need to manually round the size to a 16-byte boundary so the SSE 
// memory accesses won't GPF
#define SZ_TLLIGHT ((sizeof(TLLIGHT) + 0xf) & ~0xf)

//---------------------------------------------------------------------
// TLLIGHTING
// All the lighting related state clubbed together
//---------------------------------------------------------------------

typedef struct _TLLIGHTING
{
  // All lights
  BYTE          *pLightAlloc;     // Memory allocation for light array
  TLLIGHT       *pLightArray;     // Ptr to Growable Light array, holds all lights
  DWORD         dwLightArraySize; // Size of the light array allocated

  // Active lights
  TLLIGHT       *pActiveLights;			// linked list of the active lights
  DWORD         dwNumActiveLights;		// enabled lights, not total lights
  TLLIGHT       *pActiveLightArray[TLMAX_ACTIVE_LIGHTS];	// array of pointers to the active lights

  // Current SOA Lighting Function
  TLLIGHTFN     pfnLightVertex;
  TLLIGHTVERTEX_FUNC_TABLE LightVertexTable;


  // Temporary data used when computing lighting
  D3DVECTOR     eye_in_eye;   // eye position in eye space

  // Ma * La + Me (Ambient and Emissive) ------
  TLCOLOR       ambEmiss;
  P8RGBA		p8AmbEmissPlusDiffAlpha;	// pre-add diffuse alpha
  P8RGBA		p8AmbEmissPlusSpecAlpha;	// there is to spec emiss and spec alpha comes from diffuse alpha

  // ColorVertex stuff ------------------------
  TLCOLOR       *pAmbientSrc;     
  TLCOLOR       *pDiffuseSrc;
  TLCOLOR       *pSpecularSrc;
  TLCOLOR       *pEmissiveSrc;
  DWORD			dwAmbientSrc;					// slow path,
  DWORD			dwDiffuseSrc;					// will prolly
  DWORD			dwSpecularSrc;					// go away
  DWORD			dwEmissiveSrc;					//
  // SSE
  DWORD			pSOACvAmbientSrc;				// ptr to color vertex ambient source in TL_SOATMP
  DWORD			pSOACvDiffuseSrc;				// ptr to color vertex diffuse source in TL_SOATMP
  DWORD			pSOACvSpecularSrc;				// ptr to color vertex specular source in TL_SOATMP
  DWORD			pSOACvEmissiveSrc;				// ptr to color vertex emissive source in TL_SOATMP
  DWORD			dSOAOffsetDiffAlphaSrc;			// offset of vertex diffuse.alpha source in the input vertex
  // 3DNow
  DWORD			dwK3dOffsetCvAmbientSrc;		// offset in the TLBN of color vertex ambient source
  DWORD			dwK3dOffsetCvDiffuseSrc;		// offset in the TLBN of color vertex diffuse source
  DWORD			dwK3dOffsetCvSpecularSrc;		// offset in the TLBN of color vertex specular source
  DWORD			dwK3dOffsetCvEmissiveSrc;		// offset in the TLBN of color vertex emissive source

  // Diffuse ----------------------------------
  TLCOLOR       vertexDiffuse;     // Provided with a vertex, used if
                                   // COLORVERTEX is enabled and a diffuse
                                   // color is provided in the vertex
  TLCOLOR       fDiffuse;          // Diffuse accumulates here
  DWORD         dwDiffuse;         // Diffuse color result of lighting
  P8RGBA        p8Diffuse;         // Diffuse color result of lighting


  // Specular --------------------------------
  TLCOLOR       vertexSpecular;    // Provided with a vertex, used if
                                   // COLORVERTEX is enabled and a specular
                                   // color is provided in the vertex
  TLCOLOR       fSpecular;         // Specular accumulates here
  DWORD         dwSpecular;        // Specular color result of lighting
  P8RGBA        p8Specular;        // Specular color result of lighting

  D3DVALUE      specThreshold;     // If the dot product is less than this
                                   // value, specular factor is zero
  // End of temporary data

  // RENDERSTATEAMBIENT --------------------------------------
  // Ambient color set by D3DRENDERSTATE_AMBIENT
  // They are all scaled to 0 - 1
  D3DVALUE      ambient_red;
  D3DVALUE      ambient_green;
  D3DVALUE      ambient_blue;
  DWORD         ambient_save;   // Original unscaled color

  // Fog -----------------------------------------------------
  int           fog_mode;     // vertex fog mode
  int           fog_range_enable; // vertex fog range mode enable
  D3DCOLOR      fog_color;
  D3DVALUE      fog_density;
  D3DVALUE      fog_start;
  D3DVALUE      fog_end;
  D3DVALUE      fog_factor;   // 255 / (fog_end - fog_start)

  D3DCOLORMODEL   color_model;		// what is this???

  // Material ------------------------------------------------
  D3DMATERIAL7    Material;         // Current material         

  // Light sources
  DWORD         AmbMaterialSrc;
  DWORD         DfusMaterialSrc;
  DWORD         SpecMaterialSrc;
  DWORD         EmisMaterialSrc;

  // SOA
  SOA_DWORD     *pdSOASpecAlphaSrc;   // Spec Alpha Source (int)
  SOA_FLOAT     *pfSOASpecAlphaSrc;   // Spec Alpha Source (float)
  SOA_DWORD     *pdSOASpecRGBSrc;     // Specular RGB Src (int)

  SOA_DWORD     *pdSOADiffAlphaSrc;   // Diff Alpha source (int)
  SOA_DWORD     *pdSOADiffRGBSrc;     // Diff RGB Src (int)

  // For color material
  LPDWORD       pDiffuseAlphaSrc;
  LPDWORD       pSpecularAlphaSrc;

  DWORD         materialDiffAlpha;  // Current material diffuse
  // alpha (0-255) shifted left
  // by 24 bits

  DWORD         materialSpecAlpha;  // Current material specular
  // alpha (0-255) shifted left
  // by 24 bits
  FLOAT			fMatSpecAlpha;		// precalculate *256
  FLOAT			fMatSpecAlphaDev;	// precalculate (255-x)*256

  DWORD         vertexDiffAlpha;    // Current material diffuse
  // alpha (0-255) shifted left
  // by 24 bits

  // These are used by color vertex
  DWORD			dwMaterialAmbient;
  DWORD			dwMaterialDiffuse;
  DWORD			dwMaterialSpecular;
  DWORD			dwMaterialEmissive;

  DWORD         vertexSpecAlpha;    // Current material specular
  // alpha (0-255) shifted left
  // by 24 bits

//  D3DMATERIAL7  material;       // Cached material data
  KniPowerFunctionType KniPowerFunction;
  K3dPowerFunctionType K3dPowerFunction;

  ConvertPackedSOAColortoSOA_RGB_type  *ConvertPackedSOAColortoSOA_RGB;

  TLCOLOR      matAmb;
  TLCOLOR      matDiff;
  TLCOLOR      matSpec;
  TLCOLOR      matEmis;
} TLLIGHTING;


//-----------------------------------------------------------------------------
//
// TLVECTORH - Homogeneous vector
//
//-----------------------------------------------------------------------------
typedef struct tagTLVECTORH
{
  D3DVALUE x;
  D3DVALUE y;
  D3DVALUE z;
  D3DVALUE w;
} TLVECTORH, *PTLVECTORH;


typedef D3DMATRIX TLMATRIX;

//-----------------------------------------------------------------------------
//
// RRTRANSFORMDATA - Transformation data used by Refrence T&L implementation
// to transform vertices.
//
//-----------------------------------------------------------------------------
typedef struct tagTLTRANSFORMDATA
{
  TLMATRIX    m_PS;     // Mproj * Mshift
  TLMATRIX    m_VPS;      // Mview * PS
  TLMATRIX    m_VPSInv;   // Inverse( Mview * PS )
//  TLMATRIX    m_CTMI;     // Inverse current transformation matrix
//  TLVECTORH   m_frustum[6]; // Normalized plane equations for viewing frustum in the model space
//  DWORD      m_dwFlags;
} TLTRANSFORMDATA, *PTLTRANSFORMDATA;

//---------------------------------------------------------------------
// Digested Viewport information
// calculated from viewport settings
//---------------------------------------------------------------------
typedef struct _TL_VIEWPORTDATA
{

  D3DVALUE dvX;           // dwX
  D3DVALUE dvY;           // dwY
  D3DVALUE dvWidth;       // dwWidth
  D3DVALUE dvHeight;      // dwHeight

  // Coefficients to compute screen coordinates from normalized window
  // coordinates
  D3DVALUE scaleX;        // dvWidth/2
  D3DVALUE scaleY;        // dvHeight/2
  D3DVALUE scaleZ;        // (Viewport->dvMaxZ - Viewport->dvMinZ)
  D3DVALUE offsetX;       // dvX + scaleX
  D3DVALUE offsetY;       // dvY + scaleY
  D3DVALUE offsetZ;       // Viewport->dvMinZ
  D3DVALUE viewDataOffsetX;	  // dvX + scaleX + pixel offset
  D3DVALUE viewDataOffsetY;	  // dvY + scaleY + pixel offset

  // Coefficients to compute screen coordinates from normalized window
  // coordinates
  D3DVALUE scaleXi;       // Inverse of scaleX
  D3DVALUE scaleYi;       // Inverse of scaleY
  D3DVALUE scaleZi;       // Inverse of scaleZ

  // Min and max values for viewport window in pixels
  D3DVALUE minX;        // offsetX - scaleX
  D3DVALUE minY;        // offsetY - scaleY
  D3DVALUE maxX;        // offsetX + scaleX
  D3DVALUE maxY;        // offsetY + scaleY

  // Min and max window values with guard band in pixels
  D3DVALUE minXgb;
  D3DVALUE minYgb;
  D3DVALUE maxXgb;
  D3DVALUE maxYgb;

  // Coefficients to transform a vertex to perform the guard band clipping
  // x*gb11 + w*gb41
  // y*gb22 + w*gb42
  D3DVALUE gb11;
  D3DVALUE gb22;
  D3DVALUE gb41;
  D3DVALUE gb42;

  // Coefficients to apply clipping rules for the guard band clipping
  // They are used by clipping routines
  // w*Kgbx1 < x < w*Kgbx2
  // w*Kgby1 < y < w*Kgby2
  D3DVALUE Kgbx1;
  D3DVALUE Kgby1;
  D3DVALUE Kgbx2;
  D3DVALUE Kgby2;
} TLVIEWPORTDATA;

//---------------------------------------------------------------------
// This struct manages growing buffer, aligned to 32 byte boundary
// Number if bytes should be power of 2.
// D3DMalloc is used to allocate memory
//---------------------------------------------------------------------

// Number of clipping planes
#ifndef TLMAX_USER_CLIPPLANES
#define TLMAX_USER_CLIPPLANES 6
#endif

// Number of clipping planes
#ifndef TLMAX_CLIPPING_PLANES
#define TLMAX_CLIPPING_PLANES (6+TLMAX_USER_CLIPPLANES)
#endif

// Space for vertices generated/copied while clipping one triangle
#define TLMAX_CLIP_VERTICES   (( 2 * TLMAX_CLIPPING_PLANES ) + 3 )

// 3 verts. -> 1 tri, 4 v -> 2 t, N vertices -> (N - 2) triangles
//#define TLMAX_CLIP_TRIANGLES  ( TLMAX_CLIP_VERTICES - 2 )


typedef DWORD TLCLIPCODE;

typedef struct _TLCLIPVTX
{
  D3DVALUE    hx, hy, hz, hw;	// homogenous coordinates
  D3DVALUE    sx, sy, sz;		// screen space coordinates
  D3DVALUE    rhw;
  D3DVALUE    vfog;
  DWORD       clip_code;
  D3DCOLOR    diffuse;
  D3DCOLOR    specular;
  struct _TLCLIPVTX  *next;
  D3DVALUE    tex[D3DDP_MAXTEXCOORD*4];	// *4 for 4D textures
} TLCLIPVTX;

typedef struct _TLCLIPTRIANGLE
{
  TLCLIPVTX   *v[3];
} TLCLIPTRIANGLE;


//---------------------------------------------------------------------
// TLCLIPPING
// Cached clipping data
//---------------------------------------------------------------------
typedef struct __TLCLIPPING
{
  AlignedBuffer32  ClipBuf;
  TLCLIPVTX  *clip_vbuf1[TLMAX_CLIP_VERTICES];
  TLCLIPVTX  *clip_vbuf2[TLMAX_CLIP_VERTICES];
  TLCLIPVTX  **current_vbuf;  // clip_vbuf1 or clip_vbuf2
  TLCLIPVTX  clip_vertices[TLMAX_CLIP_VERTICES];
  DWORD      dwInterpolate;
  int        clip_vertices_used;
//  DWORD      clip_color;
//  DWORD      clip_specular;
} TLCLIPPING;

//---------------------------------------------------------------------
// Defines for dwFVFFlags member of 
//  the COMP_FVF structure below
//---------------------------------------------------------------------
#define TL_FVFFLAG_OUT      0x00000001    // Set if FVF is Output, cleared if Input
#define TL_FVFFLAG_SOA      0x00000002    // Set if FVF is SOA, cleared if AOS
//#define TL_FVFFLAG_CLIPPED  0x00000004    // Set if Using the larger Clipped Form

//---------------------------------------------------------------------
// FVF_COMP
// Composite FVF Structure
//---------------------------------------------------------------------
typedef struct __FVF_COMP
{
  DWORD   dwFVFType;        // FVF vertex type 
  DWORD   dwFVFFlags;       // Flags for variation of vertex formats
  LPVOID  lpvData;          // Pointer to vertex buffer
  DWORD   dwVStart;         // Vertex offset passed down by DP2
  DWORD   dwStride;         // amount to add to a pointer to get to the next vertex or group of vertices

// Component Info
  DWORD   dwNumBlends;      // number of blending factors.
  DWORD   dwNormalOffset; 
  DWORD   dwDiffuseOffset;  // Diffuse offset in input FVF -- Recomputed when FVF is changed.
  DWORD   dwSpecularOffset; // Specular offset in input FVF -- Recomputed when FVF is changed.
  DWORD   dwTexOffset;      // texture offset
  DWORD   dwNumTexCoords;   // number of texture coordinate sets
  DWORD   dwTexCoordSize;   // size of all texture coordinate sets
} FVF_COMP, *LPFVF_COMP;

#define SOA_PREFETCH(x) {   \
  _asm mov eax, x           \
  _asm prefetchnta [eax]    \
}


/*--------------------------------------------------------------------------------**
** This structure contains the render context data for a software transformation  **
** and lighting enabled HAL. This is added to each and every rendering context we **
** create.   These are the RDPV_xxx flags in the refrast                          **
**--------------------------------------------------------------------------------*/
#define TLPV_DOLIGHTING               0x00000001 //Lighting is enabled
#define TLPV_DOCLIPPING               0x00000002 //Clipping is enabled
#define TLPV_GUARDBAND                0x00000004 //Guardband is enabled
#define TLPV_DOFOG                    0x00000008 //Fog is enabled
#define TLPV_DOSPECULAR               0x00000010 //Specular is enabled
#define TLPV_RANGEFOG                 0x00000020 //Range fog is enabled
#define TLPV_NORMALIZENORMALS         0x00000040 //Normalize normals is enabled
#define TLPV_LOCALVIEWER              0x00000080
#define TLPV_DOCOMPUTEPOINTSIZE       0x00000100	// UNUSED!!!
//#define TLPV_DOPASSEYENORMAL          0x00000200	// WHAT IS THIS (never set)
//#define TLPV_DOPASSEYEXYZ             0x00000400	// WHAT IS THIS (never set)
#define TLPV_DOTEXGEN                 0x00000800
#define TLPV_DOTEXXFORM               0x00001000
#define TLPV_NON_2D_TEXTURE           0x00002000
#define TLPV_DO_PROSPECTIVE_DIVIDE    0x00004000  // prospective divide without texture wrapping


// ColorVertexFlags
#define TLPV_VERTEXDIFFUSENEEDED      0x00008000
#define TLPV_VERTEXSPECULARNEEDED     0x00010000
#define TLPV_COLORVERTEXAMB           0x00020000
#define TLPV_COLORVERTEXDIFF          0x00040000
#define TLPV_COLORVERTEXSPEC          0x00080000
#define TLPV_COLORVERTEXEMIS          0x00100000
#define TLPV_COLORVERTEXFLAGS ( \
				TLPV_COLORVERTEXAMB  | \
				TLPV_COLORVERTEXDIFF | \
				TLPV_COLORVERTEXSPEC | \
				TLPV_COLORVERTEXEMIS )
#define TLPV_COLORVERTEXNEEDED        0x00200000
#define TLPV_VERTEXBLENDNEEDED        0x00400000
#define TLPV_VALIDCLIPBUFFER          0x00800000 // pClipBuf pointer is valid
#define TLPV_TLNEEDED                 0x01000000 // incoming buffer requires transformation
                                                 // 0 also implies that dwFVFIn == _asm_data.vertex_type
#define TLPV_FASTPATH_OK              0x02000000 // We can use the fast path
#define TLPV_USERCLIPPLANES           0x04000000 // User clip planes are enabled
#define TLPV_DIFF_SRC_VTX             0x08000000  // Diffuse source is the vertex
#define TLPV_SPEC_SRC_VTX             0x10000000  // Specular source is the vertex
#define TLPV_NOT_SOLID_FILL_MODE      0x20000000  // point or wireframe

// Dirty Flags
#define TLPV_DIRTY_PROJXFM          0x00000001
#define TLPV_DIRTY_VIEWXFM          0x00000002
#define TLPV_DIRTY_WORLDXFM         0x00000004
#define TLPV_DIRTY_WORLD1XFM        0x00000008
#define TLPV_DIRTY_WORLD2XFM        0x00000010
#define TLPV_DIRTY_WORLD3XFM        0x00000020
#define TLPV_DIRTY_WORLDXFM_ALL	( \
                TLPV_DIRTY_WORLDXFM  | \
                TLPV_DIRTY_WORLD1XFM | \
                TLPV_DIRTY_WORLD2XFM | \
                TLPV_DIRTY_WORLD3XFM)
#define TLPV_DIRTY_EYEXFM           0x00000040
#define TLPV_DIRTY_EYE1XFM          0x00000080
#define TLPV_DIRTY_EYE2XFM          0x00000100
#define TLPV_DIRTY_EYE3XFM          0x00000200
#define TLPV_DIRTY_EYEXFM_ALL	( \
                TLPV_DIRTY_EYEXFM  | \
                TLPV_DIRTY_EYE1XFM | \
                TLPV_DIRTY_EYE2XFM | \
                TLPV_DIRTY_EYE3XFM)
#define TLPV_DIRTY_TXTRXFM          0x00000400		// any of the 8
#define TLPV_DIRTY_VIEWRECT         0x00000800
#define TLPV_DIRTY_ZRANGE           0x00001000
#define TLPV_DIRTY_XFORM ( \
                TLPV_DIRTY_PROJXFM        | \
                TLPV_DIRTY_VIEWXFM        | \
                TLPV_DIRTY_VIEWRECT       | \
                TLPV_DIRTY_WORLDXFM_ALL   | \
                TLPV_DIRTY_ZRANGE         | \
				TLPV_DIRTY_TXTRXFM        | \
                TLPV_DIRTY_DYNGB )
#define TLPV_DIRTY_MATERIAL         0x00002000
#define TLPV_DIRTY_SETLIGHT         0x00004000
#define TLPV_DIRTY_NEEDXFMLIGHT     0x00008000
#define TLPV_DIRTY_COLORVTX         0x00010000
#define TLPV_DIRTY_LIGHTING    ( \
				TLPV_DIRTY_MATERIAL     | \
                TLPV_DIRTY_SETLIGHT     | \
                TLPV_DIRTY_NEEDXFMLIGHT | \
                TLPV_DIRTY_COLORVTX)
#define TLPV_DIRTY_FOG              0x00100000
#define TLPV_DIRTY_CLIPPLANES       0x00200000
//#define TLPV_DIRTY_INVERSEWORLDVIEW 0x00400000	//unused
#define TLPV_DIRTY_FVFIN            0x00800000    //Input FVF format has changed
#define TLPV_DIRTY_FVFOUT           0x01000000    //Output FVF format has changed
#define TLPV_DIRTY_DYNGB            0x02000000    //Dynamic GB Dirty flag
#define TLPV_DIRTY_VB			    0x04000000    //Either the current VB hasn't been T&L'd or 
									    		  //a matrix, viewport, renderstate, texture, etc. 
										  		  //has changed.  If clear we don't have to T&L 
												  //the current VB, just render it.

// Device / Viewport Xform flags (dwTLDevFlags)
#define TLPV_DEV_FTOI_DIFF_RGB    0x00000001          // Convert float RGB colors to Int
#define TLPV_DEV_FTOI_SPEC_RGB    0x00000002          // Convert float RGB colors to Int
#define TLPV_DEV_COPY_DIFF        0x00000004          // Just copy diffuse color+alpha
#define TLPV_DEV_COPY_SPEC        0x00000008          // Just copy specular color+alpha
#define TLPV_DEV_ITOF_SPEC_ALPHA  0x00000010          // Compute specular alpha to float
#define TLPV_DEV_VERTEX_FOG       0x00000020          // Compute vertex fog
#define TLPV_DEV_COMBINE_DIFF     0x00000040          // Combine diff RGB + A
#define TLPV_DEV_COMBINE_SPEC     0x00000080          // Combine spec RGB + A

// Code Path's for various cpu's (dwTLCpuCodePath)
#define TL_CODEPATH_DEFAULT       0x00000000
#define TL_CODEPATH_SSE           0x00000001	// KNI or SSE1
#define TL_CODEPATH_SSE2          0x00000002	// WNI or SSE2
#define TL_CODEPATH_3DNOW         0x00000004
#define TL_CODEPATH_3DNOWEXT      0x00000008	// 3DNow with Amd MMX extenstions & SSE MMX extenstions

// vertex transform and light flags, stored in WFBI w/3dnow, tex[0].s w/SSE
#define TL_VERT_NOT_XFORMED	  	  0x00000001
#define TL_VERT_XFORMED_NOT_LIT	  0x00000002

typedef struct __TL_RENDERCONTEXT_DATA
{
  /*---------------------**
  ** State Control Flags **
  **---------------------*/
  DWORD       dwDirtyFlags;
  DWORD       dwTLState;
  DWORD       dwTLDevFlags;           // Device / Viewport Xform flags
  DWORD       dwTLBadFlags;           // Flags that control which states disable the fast path

  DWORD		  StateClipping;
  DWORD		  StateLighting;
  DWORD		  StateColorVertex;
  DWORD		  StateLocalViewer;
  DWORD		  StateNormNormals;
  DWORD		  StateVertexBlends;

  /*-------------------------------**
  ** Common FVF Buffer data     **
  **-------------------------------*/
  DWORD       dwTexCoordSize[D3DDP_MAXTEXCOORD];    // Size of each one of them in bytes
  DWORD       dwTextureCoordSizeTotal;        // In Bytes

  /*-------------------------------**
  ** Input FVF Buffer data       **
  **-------------------------------*/
  FVF_COMP      InFVF;        // Regular Input AOS FVF data structure handed to us by D3D
  FVF_COMP      SOAFVF;       // Optimized Input SOA FVF data structure we create (swizzle)
                              // The SOA is only created and used when the rendering can 
                              // run in FastPath 
  /*------------------------**
  ** Output FVF Buffer data **
  **------------------------*/
  AlignedBuffer32 TLVBuf;       // pointer to the memory allocated for the output vertex buffer

  // BobJ - 01/05/2000 For now, Setup 2 Output FVF types
  // After the Fast Path is complete, only one will be needed
  FVF_COMP      TLFVF;          // Regular Output AOS FVF data structure the slow path renders from
  FVF_COMP      TLBN;           // Optimized Output SOA FVF data structure we keep from fast path T&L

  /*----------------**
  ** Primitive Data **
  **----------------*/
  D3DPRIMITIVETYPE  primType;   // Current primitive being drawn
  DWORD         dwNumVertices;  // Number of vertices to process
  DWORD         dwNumIndices;   // Number of indices for Indexed Prims
  LPWORD        pIndices;
  AlignedBuffer32  pSynthIndices;	// index list we'll synthesize
  DWORD			prevIdxPrimVStart;	// for indexed primitives, if the VStart changes, we mark the
									// VB as dirty

  /*-------------------**
  ** USER MEM Vertices **
  **-------------------*/
  AlignedBuffer32 SOAUMBuf;     // pointer to the memory allocated for the SOA User Mem vertices

  /*---------------**
  ** Clipping data **
  **---------------*/
  DWORD         clipPlaneEnable;
  TLCLIPCODE    clipUnion;      // OR of all vertex clip flags
  TLCLIPCODE    clipIntersection; // AND of all vertex clip flags
  TLCLIPPING    clipping;
  TLVECTOR4     UserClipPlanes[TLMAX_USER_CLIPPLANES];    // User defined clipping planes  
  TLUSERCLIPPLANE xfmUserClipPlanes[TLMAX_USER_CLIPPLANES]; // User clip planes transformed
  AlignedBuffer32 ClipFlagBuf;    // memory allocation for clipping flags
  TLCLIPCODE    *pClipBuf;      // pointer to the clip flags buffer it should be == ClipFlagBuf.alignedBuf

  /*-----------------------------**
  ** Transformation Related Data **
  **-----------------------------*/
  D3DVIEWPORT7    Viewport;               // Current view port        
  TLVIEWPORTDATA  ViewData;               // Computed Viewport dependent info.
  TLTRANSFORMDATA TransformData;

  AlignedBuffer32 xfmCurrentBuf;          // pointer to the Aligned xfmCurrent Matrix
  AlignedBuffer32 xfmToEyeBuf;            // pointer to the Aligned xfmToEye Matrix
  AlignedBuffer32 xfmToEyeInvBuf;         // pointer to the Aligned xfmToEyeInv Matrix
  AlignedBuffer32 xfmToEyeInvTBuf;        // pointer to the Aligned xfmToEyeInvT Matrix
  AlignedBuffer32 xfmCurrentSOABuf;       // pointer to the Aligned SOA version of xfmCurrent Matrix
  AlignedBuffer32 xfmToEyeSOABuf;         // pointer to the Aligned SOA version of xfmToEye Matrix
  AlignedBuffer32 xfmToEyeInvTSOABuf;     // pointer to the Aligned SOA version of xfmToEyeT Matrix
  AlignedBuffer32 xfmTxtrSOABuf;     	  // pointer to the Aligned SOA version of texture transform matrices
  AlignedBuffer32 xfmTxtrTBuf;            // pointer to the Aligned SOA version of transposed texture transform matrices

  TLMATRIX        *lpxfmCurrent[TLMAX_WORLD_MATRICES];		// using WORLDi matrix 
  TLMATRIX        *lpxfmToEye[TLMAX_WORLD_MATRICES];		// Transforms to camera space (Mworld*Mview)
  TLMATRIX        *lpxfmToEyeInv[TLMAX_WORLD_MATRICES];		// and its Inverse   
  TLMATRIX        *lpxfmToEyeInvT[TLMAX_WORLD_MATRICES];	// same as above, but transposed
  SOA_MATRIX      *lpSOAxfmCurrent[TLMAX_WORLD_MATRICES];	// uses xfmWorld[];
  SOA_MATRIX      *lpSOAxfmToEye[TLMAX_WORLD_MATRICES];		// Transforms to camera space (Mword*Mview)
  SOA_MATRIX      *lpSOAxfmToEyeInvT[TLMAX_WORLD_MATRICES];	// Inverse transpose of above
  TLMATRIX        xfmProj;                  // Projection matrix
  TLMATRIX        xfmView;                  // User view matrices
  TLMATRIX        xfmWorld[TLMAX_WORLD_MATRICES];   // User world matrices
  TLMATRIX        xfmTxtr[D3DDP_MAXTEXCOORD];	// Texture transforms
  SOA_MATRIX      *lpxfmTxtrSOA[D3DDP_MAXTEXCOORD];	// SOA Texture transforms
  TLMATRIX        *lpxfmTxtrT[D3DDP_MAXTEXCOORD];		// transpose of texture transforms
  DWORD			  dwDirtyXfmTxtr;			// dirty texture transform flags (bitfield)
  int             numVertexBlends;
  BOOL            transformedVB;			// all verts have been xformed

  /*-------------------------**
  **   Lighting Related Data **
  **-------------------------*/
  TLLIGHTING      lighting;         // Lighting state         


  /*------------------------------**
  **     TL_TMP Data buffers      **
  **------------------------------*/
  AlignedBuffer32    TL_SOATMP_Buff; // Force the TL_SOATMP to be aligned on a 32 byte boundeary
  AlignedBuffer32    TL_K3DTMP_Buff; // Force the TL_K3DTMP to be aligned on a 32 byte boundeary
  TL_SOATMP          *pTL;           // pointer ot the TL_SOATMP structure.
  TL_K3DTMP			 *pTLK;			 // pointer ot the TL_K3DTMP structure

  /*-------------------------**
  **   Vertex Buffer Data
  **-------------------------*/
  LPVBSURFACEDATA   lpVBSurfData;   // pointer to current VB Surface Data structure

  /*-------------------------**
  **   Render Data 
  **-------------------------*/
  TL_RENDER_DATA    KniRC;          // Render data structure
  DWORD				dwTexOffsetIn[D3DDP_MAXTEXCOORD];
  DWORD				dwTexOffsetOut[D3DDP_MAXTEXCOORD];
  DWORD				dwTexCoordIndex[SOA_MAX_TEX];
  DWORD				dwTexXformFlags[SOA_MAX_TEX];
  TLMATRIX			*lpTexXformMatrix[SOA_MAX_TEX];
  SOA_MATRIX		*lpTexXformMatrixSOA[SOA_MAX_TEX];
  TLMATRIX			*lpTexXformMatrixT[SOA_MAX_TEX];

  /*-------------------------**
  **   Dynamic Guardband
  **-------------------------*/
  DWORD 			dwFrameCounter;
  DWORD 			dwFramesQueued;

  /*-------------------------**
  **   Special Case Functions
  **-------------------------*/
  KniSwizSingle_SwizParms_FunctionType	*KniSwizParmsFunct;
  KniSwizSingle_CopyTxtrs_FunctionType	*KniCpyTxtrFunct;
  Xform_DevCoord_Type_SOA_GEOM			*Xform_DevCoord_SOA_GEOM_Funct;
  XformDeswiz4_DevCoord_SOA_GEOM		*Xform_Deswiz4_DevCoord_SOA_GEOM_Funct;

  /*------------------------------**
  ** Code Paths for various cpu's **
  **------------------------------*/
  DWORD				dwTLCpuCodePath;

} TL_RENDERCONTEXT_DATA;



#define FVF_TRANSFORMED(dwFVF) ((dwFVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW)
#define FVF_TEXCOORD_NUMBER(dwFVF) \
  (((dwFVF) & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT)

#define TRANSFORM_NEEDED (pRc->tl.dwTLState & TLPV_TLNEEDED)
#define CLIPPING_NEEDED  (pRc->tl.dwTLState & TLPV_VALIDCLIPBUFFER)

// Check the return value and return if something wrong.
// Assume hr has been declared
#define HR_RET(exp)	\
{					\
  hr = (exp);		\
  if (hr != D3D_OK)	\
  {					\
    return hr;		\
  }					\
}

RENDERFXN_RETVAL __stdcall StateClipping(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateLighting(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateExtents(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateAmbient(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateFogVertexMode(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateColorVertex(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateLocalViewer(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateNormNormals(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateDfusMaterialSrc(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateSpecMaterialSrc(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateAmbMaterialSrc(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateEmisMaterialSrc(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateClipPlaneEnable(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateFogRangeEnable(RC *pRc, ULONG state);
RENDERFXN_RETVAL __stdcall StateVertexBlends(RC *pRc, ULONG state);

void TLHAL_InitializeNewRC(RC *pRc);
void TLHAL_DestroyRC(RC *pRc);
HRESULT ProcessPrimitive( RC *pRc );
HRESULT UpdateXformData( RC *pRc, DWORD dwXfmBlendMask);
HRESULT UpdateLightingData(RC *pRC);
HRESULT UpdateFogData(RC *pRC);
HRESULT UpdateClippingData(RC *pRC, DWORD dwClipPlanesEnable);
HRESULT SetLight(RC *pRC, TLLIGHT *pL, LPD3DLIGHT7 pLight);
HRESULT GrowLightArray(RC *pRc, const DWORD dwIndex);
HRESULT DP2TL_SetLight(RC *pRc, LPD3DHAL_DP2COMMAND pCmd, LPDWORD extra);
HRESULT DP2TL_CreateLight(RC *pRc, LPD3DHAL_DP2COMMAND pCmd);
void SetXfrm( RC *, D3DTRANSFORMSTATETYPE xfrmType, D3DMATRIX *pMat );
void SavePrimitiveData( RC *pRc, DWORD dwFVFIn, LPVOID pVtx, DWORD dwVStart, UINT cVertices, 
                D3DPRIMITIVETYPE PrimType, LPVBSURFACEDATA lpVBSurfData );
void SaveIdxPrimitiveData( RC *pRc, DWORD dwFVFIn, LPVOID pVtx, DWORD dwVStart, UINT cVertices,
                D3DPRIMITIVETYPE PrimType, LPWORD pIndices, UINT cIndices, LPVBSURFACEDATA lpVBSurfData );
void InitializeLight(TLLIGHT *pL);
BOOL LightEnable(TLLIGHT *pL, RC *pRc);
BOOL LightDisable(TLLIGHT *pL, RC *pRc);

void LightVertex(RC *pRc, D3DLIGHTINGELEMENT *pLE);
void LightVertexK3D_C(RC* pRc);
void TLLV_Directional(RC *pRc, TLLIGHT *pLight, D3DLIGHTINGELEMENT *in);
void TLLV_Point(RC *pRc, TLLIGHT *pLight, D3DLIGHTINGELEMENT *in);
void TLLV_Spot(RC *pRc, TLLIGHT *pLight, D3DLIGHTINGELEMENT *in);
void FogVertex(RC* pRc, D3DVECTOR *v, D3DLIGHTINGELEMENT *pLE, int numVertexBlends, float *pBlendFactors, BOOL bVertexInEyeSpace);
//float FogVertexK3D_C(RC* pRc);
#ifdef VCPP
void TLLV_DirectionalK3D_C(RC *pRc, TLLIGHT *pLight);
void TLLV_PointK3D_C(RC *pRc, TLLIGHT *pLight);
void TLLV_SpotK3D_C(RC *pRc, TLLIGHT *pLight);
void FogVertexK3D_Asm(RC* pRc);
__m128 taylorpowSSE(float *fexp);
#endif

HRESULT FP_IndexedTriangleList2_SOA_Split(RC *pRc);
HRESULT FP_IndexedTriangleList2_SOA_UM_Split(RC *pRc);
HRESULT FP_IndexedTriangleList2_K3D(RC *pRc);

int Inverse4x4(D3DMATRIX *src, D3DMATRIX *dst);

// KNI math functions in assembly (in tlknimath.asm)
void __cdecl KniExp();
void __cdecl Inverse4x4_KNI_Asm( D3DMATRIX *src, D3DMATRIX *dst );
void __cdecl MatrixProduct_KNI_Asm(D3DMATRIX *a, D3DMATRIX *b, D3DMATRIX* dst);
KniPowerFunctionType __cdecl UpdateKniPowerFunctionPtr(float exponent);

#ifdef VCPP
void FogVertexSOA(RC* pRc);
void Xform_DevColor_SOA(RC *pRc);
void ConvertPackedSOAColortoSOA_RGB_SSE( SOA_DWORD *src, SOA_RGB *dst);
void ConvertPackedSOAColortoSOA_RGB_SSE2( SOA_DWORD *src, SOA_RGB *dst);
void Xform_DevCoord_SOA_GEOM_WBuff(RC *pRc);
void Xform_DevCoord_SOA_GEOM_NoWBuff(RC *pRc);
void XformDeswiz4_DevCoord_SOA_GEOM_WBuff(RC *pRc, DWORD *pSrcVert, DWORD *pDstVert);
void XformDeswiz4_DevCoord_SOA_GEOM_NoWBuff(RC *pRc, DWORD *pSrcVert, DWORD *pDstVert);
void XformDeswiz4_DevCoord_SOA_GEOM_WBuff_Clipped(RC *pRc, DWORD *pSrcVert, DWORD *pDstVert);
void XformDeswiz4_DevCoord_SOA_GEOM_NoWBuff_Clipped(RC *pRc, DWORD *pSrcVert, DWORD *pDstVert);
void Xform_DeSwiz_DevCoord_SOA_TEX(RC *pRc, DWORD dwSOAIdx, SOA_UV* pTex);
//void Xform_3_SOA(SOA_XYZ *pD, SOA_XYZ *pV, SOA_MATRIX *pSOAMat4x4x4);		//UNUSED
//void Xform_4_SOA(SOA_XYZW *pD, SOA_XYZ *pV, SOA_MATRIX *pSOAMat4x4x4);		//UNUSED
void ComputeClipCodes_SOA(RC *pRc);
void FP_Xform_4Vert_SOA(RC *pRc, DWORD *pSOA);
void FP_Light_4Vert_SOA(RC *pRc, DWORD *pSOA);
void FP_XformLightK3d_Asm(RC *pRc, DWORD idx, TLBN* pDst);
void DeSwizzleSOAtoTLBN_GEOM(RC *pRc, DWORD dwSOAIdx);
void DeSwizzleSOAtoTLBNClipped_GEOM(RC *pRc, DWORD dwSOAIdx);
void SwizzleD3DtoSOA( RC *pRc, DWORD dwNumSOAVecs );
//void SwizzleSingleD3DtoSOA_C( RC *pRc, DWORD vIndx, DWORD dwSOAOffset );
void SwizzleMatrix(D3DMATRIX *pMat, SOA_MATRIX *pSOAMat4x4x4);
void AllocateSOAFVF( RC *pRc );

void SwizzleSingleD3DtoSOA_Setup( RC *pRc );
void SwizzlePartialSingleD3DtoSOA( RC *pRc, DWORD *pSrc, DWORD *pDst, DWORD numVerts );


// 3dnow math functions in assembly (in tlk3dmath.asm)
float __cdecl K3dExp(float);
void __cdecl k3dpow_amd(void);	// mmx registers are used to pass base, power, and return value
K3dPowerFunctionType __cdecl UpdateK3dPowerFunctionPtr(float exponent);
void __cdecl MatrixProduct_K3d_Asm(D3DMATRIX *a, D3DMATRIX *b, D3DMATRIX* dst);

#endif //VCPP

void TL_RENDER_Setup ( RC* pRc );
void TL_RENDER_Default_NoClip ( TL_RENDER_DATA* KniRC, lpTLBN pA, lpTLBN pB, lpTLBN pC );
void TL_RENDER_Default_Clip ( RC* pRc, lpTLBN pA, lpTLBN pB, lpTLBN pC, D3DVERTEX *pAin, D3DVERTEX *pBin, D3DVERTEX *pCin );
void TL_RENDER_Cleanup ( RC* pRc );
// Assembly version of TL_RENDER_DumpSpecBuffer (in atlrend.asm)
void __cdecl TL_RENDER_DumpSpecBuffer_Asm ( TL_RENDER_DATA* pRc );


#ifdef TNL_PROFILE
  extern void __stdcall TnLFlavour( RC *pRc, DWORD type, DWORD count );
  extern void __stdcall MatrixStats(DWORD dwxfrmtype);
  extern void __stdcall InitFVFStats(DWORD value);
  extern void __stdcall UpdateFVFStats(DWORD dwFVFFlags);
  extern void __stdcall PrintFVFStats(DWORD value);
  extern void __stdcall printTLState( DWORD value );
  extern void __stdcall printLightTypes(RC *pRc);
  extern void __stdcall printLightStates(RC *pRc);
  extern void __stdcall TriClipStats(DWORD dwClipCode);
#else
  #define TnLFlavour 1 ? (void)0 : (void)
  #define MatrixStats 1 ? (void)0 : (void)
  #define InitFVFStats 1 ? (void)0 : (void)
  #define UpdateFVFStats 1 ? (void)0 : (void)
  #define PrintFVFStats 1 ? (void)0 : (void)
  #define printTLState 1 ? (void)0 : (void)
  #define printLightTypes 1 ? (void)0 : (void)
  #define printLightStates 1 ? (void)0 : (void)
  #define TriClipStats  1 ? (void)0 : (void)
#endif



//---------------------------------------------------------------------
// ComputeTextureCoordSize:
// Computes the following device data
//  - bTextureCoordSizeTotal
//  - bTextureCoordSize[] array, based on the input FVF id
//---------------------------------------------------------------------
__inline void ComputeTextureCoordSize(DWORD dwFVF,
                    LPDWORD pdwTexCoordSizeArray,
                    LPDWORD pdwTexCoordSizeTotal)
{
  DWORD i;

  // Texture formats size  00 01   10   11
  static BYTE bTextureSize[4] = {2*4, 3*4, 4*4, 4};

  DWORD dwNumTexCoord = FVF_TEXCOORD_NUMBER(dwFVF);

  // Compute texture coordinate size
  DWORD dwTextureFormats = dwFVF >> 16;
  if (dwTextureFormats == 0)
  {

  *pdwTexCoordSizeTotal = (BYTE)dwNumTexCoord * 2 * 4;
  for (i=0 ; i < dwNumTexCoord; i++)
    pdwTexCoordSizeArray[i] = 4*2;
  }
  else
  {
  *pdwTexCoordSizeTotal = 0;
  for (i=0 ; i < dwNumTexCoord; i++)
  {
    BYTE dwSize = bTextureSize[dwTextureFormats & 3];
    pdwTexCoordSizeArray[i] = dwSize;
    *pdwTexCoordSizeTotal += dwSize;
    dwTextureFormats >>= 2;
  }
  }
  return;
} /* ComputeTextureCoordSize */

//---------------------------------------------------------------------
// GetFVFVertexSize:
//     Computes total vertex size in bytes for given fvf
//     including the texture coordinates
//---------------------------------------------------------------------
__inline DWORD
GetFVFVertexSize( DWORD qwFVF )
{
  DWORD dwNumTexCoord;
  DWORD dwTextureFormats;

  // Texture formats size  00 01   10   11
  static DWORD dwTextureSize[4] = {2*4, 3*4, 4*4, 4};

  DWORD dwSize = 3 << 2;
  switch( qwFVF & D3DFVF_POSITION_MASK )
  {
    case D3DFVF_XYZRHW: dwSize += 4;    break;
    case D3DFVF_XYZB1:  dwSize += 1*4;  break;
    case D3DFVF_XYZB2:  dwSize += 2*4;  break;
    case D3DFVF_XYZB3:  dwSize += 3*4;  break;
    case D3DFVF_XYZB4:  dwSize += 4*4;  break;
    case D3DFVF_XYZB5:  dwSize += 5*4;  break;
  }
  if (qwFVF & D3DFVF_NORMAL)
    dwSize += 3*4;
  if (qwFVF & D3DFVF_RESERVED1)
    dwSize += 4;

  if (qwFVF & D3DFVF_DIFFUSE)
    dwSize += 4;
  if (qwFVF & D3DFVF_SPECULAR)
    dwSize += 4;

  // Texture coordinates
  dwNumTexCoord = (DWORD)(FVF_TEXCOORD_NUMBER(qwFVF));
  dwTextureFormats = (DWORD)qwFVF >> 16;
  if (dwTextureFormats == 0)
  {
    dwSize += dwNumTexCoord * 2 * 4;
  }
  else
  {
    DWORD i;
    for (i=0; i < dwNumTexCoord; i++)
    {
      dwSize += dwTextureSize[dwTextureFormats & 3];
      dwTextureFormats >>= 2;
    }
  }

  return dwSize;
} /* GetFVFVertexSize */


// This is necessary because the lights need to be padded to 16-bytes.  I
// couldn't figure out a way to make the compiler do this for us automaticly.
__inline TLLIGHT* GetIndexedLightPtr(TLLIGHT *pLightArray, DWORD dwIndex)
{
	DWORD lightAddr = (DWORD)pLightArray;
	lightAddr += dwIndex * SZ_TLLIGHT;
	return (TLLIGHT*)lightAddr;
}

__inline BOOL LightIsEnabled(TLLIGHT *pL) {
    return (pL->dwFlags & TLLIGHT_ENABLED);
}


#ifdef VCPP
#define ALIGN16 __declspec(align(16))
#define ALIGN32 __declspec(align(32))
#else
#define ALIGN16
#define ALIGN32
#endif


#endif
#endif
#if( DX >= 7 )
#ifdef TnL_HAL
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
** File name: soa.h
**
** Description: SOA and AOS structures for Napalm T&L.
**
** $Revision: 36$
** $Date: 10/11/00 8:49:49 PM$
**
** $Log: 
**  36   3dfx      1.19.1.0.1.1.1.1210/11/00 Brent           Forced check in to
**       enforce branching.
**  35   3dfx      1.19.1.0.1.1.1.1110/10/00 Allen Hansen    added vertex blends to
**       the optimized paths
**  34   3dfx      1.19.1.0.1.1.1.1009/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  33   3dfx      1.19.1.0.1.1.1.909/15/00 Allen Hansen    Got rid of the
**       TLBN_CLIP structure (with homogenous coords), all output verts, clipped or
**       not, use the TLBN structure.
**  32   3dfx      1.19.1.0.1.1.1.809/13/00 Allen Hansen    Fixed bug when using
**       colorvertex; if diffuse comes from the vertex, then the alpha must come
**       from that source.  Was failing the D3DIM sample app "Shadow Volume 2"
**  31   3dfx      1.19.1.0.1.1.1.708/29/00 Allen Hansen    added colorvertex
**       support for sse path
**  30   3dfx      1.19.1.0.1.1.1.608/27/00 Allen Hansen    added colorvertex
**       values to TL_K3DTMP
**  29   3dfx      1.19.1.0.1.1.1.508/21/00 Allen Hansen    removed "__m128" from
**       SOA unions, it was causing the parent stuctures to be 16-byte padded (this
**       caused a really tough-to-find bug!)
**  28   3dfx      1.19.1.0.1.1.1.407/08/00 Allen Hansen    replaced hard-wired
**       TLBN and TLBN_CLIP size macros with SIZE xxx
**  27   3dfx      1.19.1.0.1.1.1.307/05/00 Allen Hansen    added guardband params
**       to TL_K3DTMP (they needed to be 8-byte aligned)
**  26   3dfx      1.19.1.0.1.1.1.207/03/00 Allen Hansen    added support for 3dnow
**       special cased asm lighting and transformation functions
**  25   3dfx      1.19.1.0.1.1.1.106/29/00 Allen Hansen    TL_K3DTMP: added
**       xfmCurrentT & xfmToEyeT transposed matricies, other minor cleanup
**  24   3dfx      1.19.1.0.1.1.1.006/25/00 Allen Hansen    renamed the TL_TMP
**       structure to TL_SOATMP, since it's strictly SOA, added TL_K3DTMP structure
**       for 3dnow
**  23   3dfx      1.19.1.0.1.106/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  22   3dfx      1.19.1.0.1.005/30/00 Allen Hansen    Changed the parameter order
**       in the TLBN and TLBN_CLIPPED structures
**  21   Napalm R1 1.19.1.0    04/29/00 Bob Johnston    Added missing members of
**       TL_TMP
**  20   Napalm    1.19        04/11/00 Bob Johnston    Optimization's in the
**       Device Coordinate Transform code for retrieving scales and offsets in SOA
**       form, thus minimizing the only fly shuffle and HW offset adds.
**  19   Napalm    1.18        03/23/00 Allen Hansen    Functional assembly version
**       of the SW T&L rendering code
**  18   Napalm    1.17        03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  17   Napalm    1.16        03/14/00 Scott Kephart   Added support for
**       Xform_DevColor_SOA
**  16   Napalm    1.15        03/03/00 Scott Kephart   SW T&L: re-wrote
**       DeSwizzleSOAToTLBN This allows  us to change the ordering of members in
**       the TLBN struct without breaking code. 
**       Added 1 dword of padding to TLBN. Moved clip_code to before the texture
**       coordinates in TLBN_CLIP. (Same position as pad DWORD in TLBN).
**  15   Napalm    1.14        03/01/00 Allen Hansen    Fixed vertex fog for T&L
**  14   Napalm    1.13        02/28/00 Scott Kephart   Lighting fixes. Fixed
**       specular on point lights, added support for dvFalloff on spotlights.
**  13   Napalm    1.12        02/22/00 Bob Johnston    Fastpath rendering with
**       non-clipped vertices.  Texture coordinate handleing bug fixes, lots of bug
**       fixes.  Still showing some bugs in the fastpath.
**  12   Napalm    1.11        02/21/00 Bob Johnston    Optimized Viewport
**       transform's use of texture coords and made changes in the swizzle of Text
**       coords into SOAFVF.  Also added support in the vertex proceesing loop to
**       call on the newly added fast path rendering code.  Added necessary changes
**       to the T&L context data structure to support rendering
**  11   Napalm    1.10        02/18/00 Scott Kephart   Added writemask to TL_TMP
**  10   Napalm    1.9         02/16/00 Scott Kephart   Added more elements to
**       TL_TMP for lighting
**  9    Napalm    1.8         02/16/00 Bob Johnston    Completed full viewport
**       transformation code to spec.  Updated test code to use new viewport xform.
**  8    Napalm    1.7         02/15/00 Scott Kephart   Updates for SOA lighting
**  7    Napalm    1.6         02/10/00 Scott Kephart   Data structure cleanup for
**       SOA.H -- we're unionized now!
**  6    Napalm    1.5         02/08/00 Scott Kephart   Turn on test code. 
**  5    Napalm    1.4         02/07/00 Scott Kephart   Added new data structs to
**       support clip code generation
**  4    Napalm    1.3         02/03/00 Scott Kephart   added wfbi to TLBN and
**       TL_TMP
** 
**  3    Napalm    1.2         02/01/00 Scott Kephart   More lighting changes.
**       Better SSE matrix multiply code.
**  2    Napalm    1.1         01/28/00 Scott Kephart   Big T&L Merge: SSE /
**       fastpath data structure definitions
**  1    Napalm    1.0         12/13/99 Scott Kephart   
** $
** 
** 7     3/12/00 5:19p Skephart
** Organized data in TL_SOATMP
** 
** 6     3/10/00 4:43p Skephart
** Rearrange color data / TL_SOATMP data into device independent 
** and device dependent parts
** 
** 5     3/08/00 9:24p Skephart
 * 
 * 5     1/27/00 12:43a Skephart
 * New changes from Bob, Joe, Scott
 * 
 * 4     1/19/00 2:24p Skephart
 * Bob's FVF_COMP changes part II
*/



// 
// AOS Structures
//
// AOS structures are used by the rendering code, which must deal with triangles.
// Consequently, it is better for the rendering code for the vertex data to be 
// present in AOS order.
typedef struct __AOS_ST
{
  float s;
  float t;
}  AOS_ST;

typedef struct __AOS_UV
{
  float u;
  float v;
}  AOS_UV;



#define NAPALM_MAX_TEXTURES 2

//
// TL_BUFF structures for Napalm
//

// This is the structure for the TL_BUFF for Napalm
// 
// Data in this structure has been transformed and lit.
//
// The texture coordinates are present in the order that they will be passed
// to the hardware, which is NOT necessarily the order in which they were 
// presented in the original FVF vertex.

// 48 bytes -- 1.5 cachelines on a PIII, .75 cachelines on Willamette
typedef struct __TLBN
{
  float x, y, z;                    // x,y,z in device coordinates
  float wfbi;                       // w coordinate for w-buffering
  float w;                          // 1/w
  DWORD clip_code;                  // vertex clipping code
  DWORD diffuse, specular;          // diffuse & specular colors
  AOS_ST tex[NAPALM_MAX_TEXTURES];  // Array of two textures
} TLBN, *lpTLBN;

#define TLBN_SIZE sizeof(TLBN)




// SOA Structures
//
// These structures are used by the T&L code for the Pentium III. 
//
// Data is arranged in SOA (Structure of Array) format to facilitate 
// T&L with SSe registers. 
// 
// Two versions of the following structures are presented -- one 
// for use with the Intel compiler, and one that's compatible with
// MSVC.

#define SOA_SIZE  4
#define SOA_MAX_TEX		NAPALM_MAX_TEXTURES

// Number of world matrices
#define TLMAX_BLEND_WEIGHTS   (D3DVBLEND_3WEIGHTS+1)	// 4
#define TLMAX_WORLD_MATRICES  TLMAX_BLEND_WEIGHTS

#define FVF_OFFSET_BLEND0	((3 + 0) * (SIZE D3DVALUE))	// blend between 2 world matrices
#define FVF_OFFSET_BLEND1	((3 + 1) * (SIZE D3DVALUE))	// blend between 3 world matrices
#define FVF_OFFSET_BLEND2	((3 + 2) * (SIZE D3DVALUE))	// blend between 4 world matrices
#define FVF_OFFSET_BLEND3	((3 + 3) * (SIZE D3DVALUE))	// blend between 5 world matrices (unsupported by DX)
#define FVF_OFFSET_BLEND4	((3 + 4) * (SIZE D3DVALUE))	// blend between 6 world matrices (unsupported by DX)
#define FVF_OFFSET_BLEND5	((3 + 5) * (SIZE D3DVALUE))	// blend between 7 world matrices (unsupported by DX)


// Define the basic units for SOA data -- SOA_FLOAT and SOA_DWORD
// These are small arrays (4 elements) of either floats or DWORDS
// In these unions, f = float, m = mmx (64 bit), and x = XMM (128 bit)

// Two MMX words
typedef struct __MMX2 {
  __int64 lo;
  __int64 hi;
} MMX2;

typedef union __SOA_FLOAT {
  float     f[SOA_SIZE];        // 4 floats viewed as array of 4 32 bit values
  MMX2      m;                  // 4 floats viewed as array of 2 64 bit values
} SOA_FLOAT;

typedef union __SOA_DWORD {
  DWORD     d[SOA_SIZE];        // 4 DWORDS viewed as array of 4 32 bit values
  MMX2      m;      			// 4 DWORDS viewed as array of 2 64 bit values
} SOA_DWORD;


typedef struct __SOA_MATRIX
{
  SOA_FLOAT _11, _12, _13, _14; 
  SOA_FLOAT _21, _22, _23, _24; 
  SOA_FLOAT _31, _32, _33, _34; 
  SOA_FLOAT _41, _42, _43, _44; 
} SOA_MATRIX; 


typedef struct __SOA_RGBA
{
  SOA_FLOAT red;
  SOA_FLOAT green;
  SOA_FLOAT blue;
  SOA_FLOAT alpha;
} SOA_RGBA;

typedef struct __SOA_RGB
{
  SOA_FLOAT red;
  SOA_FLOAT green;
  SOA_FLOAT blue;
} SOA_RGB;

// SOA_UV
// UV coordinates require some explanation. The UV coordinates are not
// swizzled into SOA form. They are swizzled SOA into u, v pairs ordered by
// texture stage. So we have:
// Tex Stage 0: u, v, u, v, u, v, u, v (u and v pairs for vertices 0 - 3)
// Tex Stage 1: u, v, u, v, u, v, u, v (u and v pairs for vertices 0 - 3)

typedef union __SOA_UV
{
  AOS_UV f[SOA_SIZE];
  __int64 m[SOA_SIZE];
} SOA_UV; 

// SOA_XYZW
// 4-D coordinates used for various things.
 typedef struct __SOA_XYZW
{
  SOA_FLOAT x;
  SOA_FLOAT y;
  SOA_FLOAT z;
  SOA_FLOAT w;
} SOA_XYZW;

// SOA_XYZ
// 3-D coordinates used for various things.
typedef struct __SOA_XYZ
{
  SOA_FLOAT x;
  SOA_FLOAT y;
  SOA_FLOAT z;
} SOA_XYZ;

// SOA_XY
// 2-D coordinates used for various things.
typedef struct __SOA_XY
{
  SOA_FLOAT x;
  SOA_FLOAT y;
} SOA_XY;


// 64-bit packed integer versions of colors in Napalm device specific order (bgra)
typedef union __P8RGBA
{
  __int64	qw;
  struct {
     signed short b,g,r,a;
  } sw;
  struct {
     unsigned short b,g,r,a;
  } uw;
  struct {
     WORD b,g,r,a;
  } w;
} P8RGBA;


// TL_SOATMP
// This is the temporary reults buffer for the SOA T&L code. This holds the 
// results from transformation, clip check, lighting, viewport transform, and
// conversion of coordinates into hardware format. TL_SOATMP is not a very tidy 
// structure. It contains many disparate elements, all that need to be aligned
// to 16 byte boundaries. It's setup this way so that only a single pointer 
// dereference is needed to access the SW T&L SOA Data. This is especially 
// convenient for assembly code, where using a single base register is a great
// savings.
typedef struct __TL_SOATMP
{
  // Transformation and Lighting Output Data
  // This data is not Device Specific
  // ---------------------------------------------------------------------------
  // Geometry output
  SOA_XYZW  SOAhv;                               // homogeneous (clip space) vertex
  SOA_DWORD dSOAclip_code;                       // clip codes for vertices
  SOA_XYZ   SOAcv;                               // position in eye (camera) space 
  SOA_XYZ   SOAcn;                               // normal in eye (camera) space

  // Lighting Output
  SOA_RGB   fSOADiffuseOut;                      // diffuse RGB color as a float
  SOA_RGB   fSOASpecularOut;                     // specular RGB color as a float

  // Lighting Input (input vertex colors used by colorvertex)
  SOA_RGB   fSOAAmbientIn;						 // ambient color (world ambient)
  SOA_RGB   fSOADiffuseIn;                       // diffuse RGB color as a float
  SOA_RGB   fSOASpecularIn;                  	 // specular RGB color as a float
  SOA_DWORD	dSOADiffAlpha;						 // only the alpha of the diffuse

  // ---------------------------------------------------------------------------

  // Device Specific Data starts here
  // ---------------------------------------------------------------------------
  // Napalm Device Specific (computed by Xform_DevCoord_SOA and swizzled out to TLBN.
  SOA_XYZW  SOAsv;                               // vertex in device coordinates
  SOA_FLOAT fSOAwfbi;                            // W coordinate for W buffering
  SOA_FLOAT fSOAVertFog;                         // float version of specular alpha needed for fog computation
  SOA_DWORD dSOAdiff;                            // diffuse color
  SOA_DWORD dSOAspec;                            // specular color  
  SOA_UV    SOAtex[SOA_MAX_TEX];                 // room for up to 2 texture coordinates
  SOA_FLOAT fSOAcumulBlend;                      // blend accumulation
  // ---------------------------------------------------------------------------


  // ---------------------------------------------------------------------------
  // Data used by Xform_DevColor_SOA
  // Intermediate results for lighting data in Napalm format
  // Lighting 
  SOA_DWORD dSOADiffuse;                         // diffuse RGB color as a dword
  SOA_DWORD dSOASpecular;                        // specular RGB color as a dword

  // ---------------------------------------------------------------------------
  // Fog
  SOA_FLOAT fSOAFog;                             // Alpha value for Fog as a float
  SOA_DWORD dSOAFog;                             // Alpha value for Fog as an int

  // ---------------------------------------------------------------------------
  // Material
  SOA_RGB   fSOAmatDiffuse;
  SOA_RGB   fSOAmatSpecular;
  SOA_RGB   fSOAmatAmbient;
  SOA_RGB   fSOAmatEmissive;
  SOA_FLOAT fSOAMatSpecAlpha;                    // Material Specular Alpha as a float
  SOA_DWORD dSOAMatSpecAlpha;                    // Material Specular Alpha as an int
  SOA_DWORD dSOAMatDiffAlpha;                    // Material Diffuse Alpha as an int

  // ---------------------------------------------------------------------------
  // Misc. SOA T&L temporaries - These are used internally by various routines

  // ---------------------------------------------------------------------------
  // Clip Check
  // guardband temps
  SOA_FLOAT fSOAgb11;                             // (1.1) in xform matrix for gb clip
  SOA_FLOAT fSOAgb22;                             // (2,2) in xform matrix for gb clip
  SOA_FLOAT fSOAgb41;                             // (4,1) in xform matrix for gb clip
  SOA_FLOAT fSOAgb42;                             // (4,2) in xform matrix for gb clip
  SOA_XY    SOAgb_v;                              // Vertex transformed for guardband clip check

  // User clip plane temps
  SOA_FLOAT fSOAuser_planeeq;                     // result of user clip plane equation
  // ---------------------------------------------------------------------------

  // Lighting
  // ---------------------------------------------------------------------------
  SOA_RGBA  fSOAambEmiss;                         // Ma * La + Me (Ambient and Emissive)

  // Texture Coord Info in SOA form
  // ---------------------------------------------------------------------------
  SOA_FLOAT fSOAscaleST0;                         // duplicated ScaleS and ScaleT for Tex 0
  SOA_FLOAT fSOAcenterST0;                        // duplicated centerS and centerT for Tex 0
  SOA_FLOAT fSOAscaleST1;                         // duplicated ScaleS and ScaleT for Tex 1
  SOA_FLOAT fSOAcenterST1;                        // duplicated centerS and centerT for Tex 0

  // Transformation
  // Dev Coord Info in SOA Form
  SOA_FLOAT fSOAviewDataScaleX;                   // Broadcasted View data Scale X
  SOA_FLOAT fSOAviewDataScaleY;                   // Broadcasted View data Scale Y
  SOA_FLOAT fSOAviewDataScaleZ;                   // Broadcasted View data Scale Z
  SOA_FLOAT fSOAviewDataOffsetX;                  // Broadcasted View data Offset X
  SOA_FLOAT fSOAviewDataOffsetY;                  // Broadcasted View data Offset Y
  SOA_FLOAT fSOAviewDataOffsetZ;                  // Broadcasted View data Offset Z
  SOA_FLOAT fSOAhwZScale;                         // Broadcasted HW Zscale
  SOA_FLOAT fSOAwbuffScale;                       // Broadcasted W buffer Scale
  SOA_FLOAT fSOAwbuffOffset;                      // Broadcasted W buffer Offset
   
} TL_SOATMP;

#define SIZE_TL_SOATMP sizeof(TL_SOATMP)


// pointer to transform function
typedef void (*XFORMFNP)(RC *pRc, DWORD *pSrc, TLBN *pDst, DWORD numVerts);

typedef struct __TL_K3DTMP
{
  // ---------------------------------------------------------------------------
  // Aligned shared data
  // ---------------------------------------------------------------------------
  D3DMATRIX		xfmCurrentT[TLMAX_WORLD_MATRICES];	// transpose of pRc->tl.lpxfmCurrent[]
  D3DMATRIX		xfmToEyeT[TLMAX_WORLD_MATRICES];	// transpose of pRc->tl.lpxfmToEye[]
  D3DMATRIX		lpxfmToEyeInv[TLMAX_WORLD_MATRICES]; // aligned copy of pRc->tl.lpxfmToEyeInv[]

  AOS_UV		TxScale[SOA_MAX_TEX];				// from pRc->sst.scaleS/T
  AOS_UV		TxCenter[SOA_MAX_TEX];				// from pRc->sst.centerS/T

  // ---------------------------------------------------------------------------
  // Transformation and Lighting Output Data
  // This data is not Device Specific
  // ---------------------------------------------------------------------------
  // Geometry output
  TLVECTOR4		hv;									// homogenous coordinates
  D3DVECTOR 	cv;                               	// position in eye (camera) space (aligned)
  D3DVECTOR 	cn;                               	// normal in eye (camera) space (aligned)
  XFORMFNP		K3dXformFnp;						// Pointer to the special-cased xform function

  // Colorvertex stuff
  P8RGBA		p8Ambient_U12pt0;					// ambient light << 4
  P8RGBA		p8AmbientSrc_U12pt0; 				// ambient from the vertex or material << 4
  P8RGBA		p8DiffuseSrc_U12pt0; 				// diffuse from the vertex or material << 4
  P8RGBA		p8SpecularSrc_U16pt0;				// specular from the vertex or material	<< 8
  P8RGBA		p8EmissiveSrc_U8pt0; 				// emmissive from the vertex or material (no shift)

  // ---------------------------------------------------------------------------
  // Device Specific Data starts here
  // ---------------------------------------------------------------------------
  // Napalm Device Specific
  D3DVALUE      gb11, gb22, gb41, gb42;				// guardband (aligned)
  D3DVALUE		viewDataScaleZ;						// we'll pre-calculate ViewData.scaleZ * zScale
  D3DVALUE		viewDataOffsetZ;					// we'll pre-calculate ViewData.offsetZ * zScale
  DWORD			pad2, pad3;

} TL_K3DTMP;



// Shorthand versions of type names to make inline assember easier to write
typedef SOA_MATRIX    _SM,  *_PSM;            //XMM Matrix
typedef SOA_XYZW      _SV4, *_PSV4;           //XMM 4D Vector
typedef SOA_XYZ       _SV3, *_PSV3;           //XMM 3D Vector
typedef SOA_XY        _SV2, *_PSV2;           //XMM 2D Vector
typedef TL_SOATMP     _STL, *_PSTL;           //XMM TL_SOATMP structure



#endif
#endif
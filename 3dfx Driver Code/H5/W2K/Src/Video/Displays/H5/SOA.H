#if( DX >= 7 )
#ifdef TnL_HAL
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
** File name: soa.h
**
** Description: SOA and AOS structures for Napalm T&L.
**
** $Revision: 21$
** $Date: 10/11/00 8:45:24 PM$
**
** $Log: 
**  21   3dfx      1.19.2.0    10/11/00 Brent           Forced check in to enforce
**       branching.
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
** Organized data in TL_TMP
** 
** 6     3/10/00 4:43p Skephart
** Rearrange color data / TL_TMP data into device independent 
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

// This is the structure for the TL_BUFF for Napalm when the render state 
// is set for no clipping. This structure is a subset of TLBN_CLIP
// 
// Data in this structure has been transformed and lit.
//
// The texture coordinates are present in the order that they will be passed
// to the hardware, which is NOT necessarily the order in which they were 
// presented in the original FVF vertex.

// 48 bytes -- 1.5 cachelines on a PIII, .75 cachelines on Willamette
typedef struct __TLBN
{
  float z;                          // z in device coordinates
  float wfbi;                       // w coordinate for w-buffering
  float x, y;                       // x,y in device coordinates
  float w;                          // 1/w
  DWORD rsvd1;                      // dummy to pad things to 16 byte boundary. (Thanks Allen!)
  DWORD diffuse, specular;          // diffuse & specular colors
  AOS_ST tex[NAPALM_MAX_TEXTURES];  // Array of two textures
} TLBN, *lpTLBN;


#define TLBN_SIZE sizeof(TLBN)


// This is the structure for the TL_BUFF for Napalm when the render state 
// is set for clipping. 
// 
// Data in this structure has been transformed and lit.
//
// The texture coordinates are present in the order that they will be passed
// to the hardware, which is NOT necessarily the order in which they were 
// presented in the original FVF vertex.
//
// hx, hy, hz, hw are x, y, z, and w in homogeneous coordinates. These are 
// computed as a byproduct of clip code generation, and they are used by
// the clipping code. So we leave them in the vertex, to reduce the amount
// of data that the clipping code must recompute.

//64 bytes -- 2 cachelines on a PIII, 1 cacheline on Willamette
typedef struct __TLBN_CLIP
{
  float z;                          // z in device coordinates
  float wfbi;                       // w coordinate for w-buffering
  float x, y;                       // x,y in device coordinates
  float w;                          // 1/w
  DWORD clip_code;                  // vertex clipping code
  DWORD diffuse, specular;          // diffuse & specular colors
  AOS_ST tex[NAPALM_MAX_TEXTURES];  // Array of two textures
  float hx;                         // homogeneous x
  float hy;                         // homogeneous y
  float hz;                         // homogeneous z
  float hw;                         // homogeneous w
} TLBN_CLIP, *lpTLBN_CLIP;

#define TLBN_CLIP_SIZE sizeof(TLBN_CLIP)








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
#define SOA_MAX_TEX 2
#define SOA_TEX_COORD 2

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
#ifdef SSECPP      
  __m128    x;                  // 4 floats viewed as 1 128 bit value
#endif

} SOA_FLOAT;

typedef union __SOA_DWORD {
  DWORD     d[SOA_SIZE];        // 4 DWORDS viewed as array of 4 32 bit values
  MMX2      m;      // 4 DWORDS viewed as array of 2 64 bit values
#ifdef SSECPP
  __m128    x;                  // 4 DWORDS viewed as 1 128 bit value
#endif
} SOA_DWORD;

// SOA_MATRIX
// SOA format matrix

typedef struct __SOA_MATRIX
{
  SOA_FLOAT _11, _12, _13, _14; 
  SOA_FLOAT _21, _22, _23, _24; 
  SOA_FLOAT _31, _32, _33, _34; 
  SOA_FLOAT _41, _42, _43, _44; 
} SOA_MATRIX; 


// SOA_RGBA 
// floating point versions of colors.
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
#ifdef SSECPP
  __m128  x[2];
#endif  
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

// TL_TMP
// This is the temporary reults buffer for the SOA T&L code. This holds the 
// results from transformation, clip check, lighting, viewport transform, and
// conversion of coordinates into hardware format. TL_TMP is not a very tidy 
// structure. It contains many disparate elements, all that need to be aligned
// to 16 byte boundaries. It's setup this way so that only a single pointer 
// dereference is needed to access the SW T&L SOA Data. This is especially 
// convenient for assembly code, where using a single base register is a great
// savings.


typedef struct __TL_TMP
{
  // Transformation and Lighting Output Data
  // This data is not Device Specific
  // ---------------------------------------------------------------------------
  // Geometry output
  SOA_XYZW hv;                                // homogeneous (clip space) vertex
  SOA_DWORD clip_code;                        // clip codes for vertices
  SOA_XYZ cv;                                 // position in eye (camera) space 
  SOA_XYZ cn;                                 // normal in eye (camera) space

  // Lighting Output
  SOA_RGB fDiffuse;                           // diffuse RGB color as a float
  SOA_RGB fSpecular;                          // specular RGB color as a float
  // ---------------------------------------------------------------------------

  // Device Specific Data starts here
  // ---------------------------------------------------------------------------
  // Napalm Device Specific (computed by Xform_DevCoord_SOA and swizzled out 
  // to TLBN.
  SOA_XYZW sv;                                // vertex in device coordinates
  SOA_FLOAT wfbi;                             // W coordinate for W buffering
  SOA_FLOAT fVertFog;                         // float version of specular alpha
                                              // needed for fog computation
  SOA_DWORD diff;                             // diffuse color
  SOA_DWORD spec;                             // specular color  
  SOA_UV tex[SOA_MAX_TEX];                    // room for up to 2 texture coordinates
  // ---------------------------------------------------------------------------


  // ---------------------------------------------------------------------------
  // Data used by Xform_DevColor_SOA
  // Intermediate results for lighting data in Napalm format
  // Lighting 
  SOA_DWORD dDiffuse;                         // diffuse RGB color as a dword
  SOA_DWORD dSpecular;                        // specular RGB color as a dword

  // Fog
  SOA_FLOAT fFog;                             // Alpha value for Fog as a float
  SOA_DWORD dFog;                             // Alpha value for Fog as an int


  // Material
  SOA_FLOAT fMatSpecAlpha;                    // Material Specular Alpha as a float
  SOA_DWORD dMatSpecAlpha;                    // Material Specular Alpha as an int
  SOA_DWORD dMatDiffAlpha;                    // Material Diffuse Alpha as an int
  // ---------------------------------------------------------------------------


  // Misc. SOA T&L temporaries - These are used internally by various routines


  // Transformation

  // ---------------------------------------------------------------------------
  // Clip Check
  // guardband temps
  SOA_FLOAT gb11;                             // (1.1) in xform matrix for gb clip
  SOA_FLOAT gb22;                             // (2,2) in xform matrix for gb clip
  SOA_FLOAT gb41;                             // (4,1) in xform matrix for gb clip
  SOA_FLOAT gb42;                             // (4,2) in xform matrix for gb clip
  SOA_XY gb_v;                                // Vertex transformed for guardband clip check

  // User clip plane temps
  SOA_FLOAT user_planeeq;                     // result of user clip plane equation
  // ---------------------------------------------------------------------------

  // Lighting

  // ---------------------------------------------------------------------------
  SOA_RGBA fambEmiss;                         // Ma * La + Me (Ambient and Emissive)
  SOA_DWORD writemask;                        // writemask for lighting
  SOA_FLOAT dot;                              // dot product
  SOA_FLOAT att;                              // attenuation factor
  SOA_FLOAT dist;                             // distance
  SOA_XYZ halfway;                            // halfway vector
  SOA_XYZ eye;                                // incident vector -- vector from eye
  SOA_XYZ d;                                  // direction vector from vertex to light
  SOA_DWORD cmpres;
  // ---------------------------------------------------------------------------

  // Texture Coord Info in SOA form
  SOA_FLOAT scaleST0;                         // duplicated ScaleS and ScaleT for Tex 0
  SOA_FLOAT centerST0;                        // duplicated centerS and centerT for Tex 0
  SOA_FLOAT scaleST1;                         // duplicated ScaleS and ScaleT for Tex 1
  SOA_FLOAT centerST1;                        // duplicated centerS and centerT for Tex 0

  // Dev Coord Info in SOA Form
  SOA_FLOAT viewDataScaleX;                   // Broadcasted View data Scale X
  SOA_FLOAT viewDataScaleY;                   // Broadcasted View data Scale Y
  SOA_FLOAT viewDataScaleZ;                   // Broadcasted View data Scale Z
  SOA_FLOAT viewDataOffsetX;                  // Broadcasted View data Offset X
  SOA_FLOAT viewDataOffsetY;                  // Broadcasted View data Offset Y
  SOA_FLOAT viewDataOffsetZ;                  // Broadcasted View data Offset Z
  SOA_FLOAT hwZScale;                         // Broadcasted HW Zscale
   
} TL_TMP;

#define SIZE_TL_TMP sizeof(TL_TMP)

// Definitions for cmpps
#define SSE_EQ  0           // ==
#define SSE_LT  1           // <
#define SSE_LE  2           // <=
#define SSE_NE  4           // !=
#define SSE_NLT 5           // ! <
#define SSE_NLE 6           // ! <= 
#define SSE_GE  SSE_NLT     // >=             // added these last two AllenH 3/2/00
#define SSE_GT  SSE_NLE     // >

// Shorthand versions of type names to make inline assember easier to write

typedef SOA_MATRIX    _SM,  *_PSM;            //XMM Matrix
typedef SOA_RGBA      _SC4, *_PSRGBA;         //XMM Color 4
typedef SOA_RGB       _SC3, *_PSRGB;          //XMM Color 3
typedef SOA_UV        _SUV, *_PSUV;           //XMM UV
typedef SOA_XYZW      _SV4, *_PSV4;           //XMM 4D Vector
typedef SOA_XYZ       _SV3, *_PSV3;           //XMM 3D Vector
typedef SOA_XY        _SV2, *_PSV2;           //XMM 2D Vector
typedef TL_TMP        _STL, *_PSTL;           //XMM TL_TMP structure



#endif
#endif
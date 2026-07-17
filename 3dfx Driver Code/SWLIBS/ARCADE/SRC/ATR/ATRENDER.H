/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
**
** $Revision: 4$ 
** $Date: 10/11/00 7:33:58 PM$ 
**
*/

#ifndef _ATRENDER_H_
#   define _ATRENDER_H_

/*
**------------------------------------------------------------------------
** TYPES
**------------------------------------------------------------------------
*/

#include <3dfx.h>
#include <atutil.h>
#include <atmath.h>
#include <stdlib.h>
#include <stdio.h>

BEGINFDECL()

/*
**------------------------------------------------------------------------
** Misc Types
**------------------------------------------------------------------------
*/


typedef FxU32 AtrBuffer;
#define ATR_BUFFER_FRONTBUFFER 0x0
#define ATR_BUFFER_BACKBUFFER  0x1
#define ATR_BUFFER_AUXBUFFER   0x2

/*-------------------------------------------------------------------
  Structure: AtrDriverCaps
  Date: 6/11
  Implementor(s): jdt
  Library: AT Render
  Description:
    Describes installed graphics subsystem.
  Members:
    device         - what kind of device are we driving
    pfxRev         - revision number of pixelFx chip
    pfxMem         - number of megabytes of framebuffer memory
    numTex         - number of texelFx chips available
    tfxRev         - revision number of texelFx chips
    tfxMem         - number of megabytes of texelFx memory
    sli            - True if SLI is enabled
    texEntrySize   - size of a texture entry in bytes
    fullScreen     - driver requires fullScreen operation
    x              - x location of window
    y              - y location of window
    width          - width of window
    width          - height of window
    bpp            - depth of window
  -------------------------------------------------------------------*/

#define ATB_DEVICE_GLIDE 0
#define ATB_DEVICE_D3D   1

typedef struct {
  int    tfxRev;  
  int    tfxMem;
} AtrTFXConfig;

#define ATR_NUM_TFX 2

typedef struct {
    FxU32        deviceType;
    FxU32        pfxRev;                   
    FxU32        pfxMem;                   
    FxU32        numTex;                   
    AtrTFXConfig tfxConfig[ATR_NUM_TFX];
    FxBool       sli;
    FxBool       hasDepth;
    FxBool       fullScreen;
    FxI32        x;
    FxI32        y;
    FxU32        width;
    FxU32        height;
    FxU32        bpp;
} AtrDriverCaps;

/*-------------------------------------------------------------------
  Structure: AtrColor
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Color structure.  All colors in ATB are represented as
    floating point 0-1.0 normalized.
  Members:
    r - 0-1 floating point red
    g - 0-1 floating point green
    b - 0-1 floating point blue
  -------------------------------------------------------------------*/
typedef struct { 
    float r;
    float g;
    float b;
} AtrColor;

/*
**------------------------------------------------------------------------
** VERTEX DATA STRUCTURE
**------------------------------------------------------------------------
*/

 /*-------------------------------------------------------------------
  Structure: GrVertex
  Date: 3/19/96
  Implementor(s): da
  Library: AT Render
  Description: For Backward compatibility between Glide3 and Glide2
  we need to have a GrVertex...
  Members:
  -------------------------------------------------------------------*/
#ifdef GLIDE3
typedef struct {
    float sow;
    float tow;
    float oow;
} AtrGrTmuVertex;

typedef struct {
    float x, y, z;
    float r, g, b;
    float ooz;
    float a;
    float oow;
    AtrGrTmuVertex tmuvtx[ATR_NUM_TFX];
} GrVertex;
#endif

 /*-------------------------------------------------------------------
  Structure: AtrDstVertex
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Destination Vertex for Rendering Ops.  Used in Rendering pipes
    N.B. This MUST match the layout of GrVertex.
  Members:
    x, y       - screen space x and y
    flags      - space for clipping codes
    ooz        - used for z buffering systems 
    oow        - one over w
    r, g, b, a - color for itrgb
    flag       - vertex processing flags ( eg. clip codes )
    s0, t0     - texture0 coordinates
    s1, t1     - texture1 coordinates
    pad0       - pads data structure out to 64 bytes
  -------------------------------------------------------------------*/
typedef struct {
    float x, y;
    FxU32 flags;
    float r, g, b;
    float ooz;
    float a;
    float oow;
    float s0, t0, oow0;
    float s1, t1, oow1;
    float pad0;
} AtrDstVertex;

/*-------------------------------------------------------------------
  Structure: AtrVertex
  Date: 3/18/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Fundamental Vertex Structure.  Localizes all common per vertex 
    information. Include 3 sets of general purpose texture coordinates
  Members:
    x,y,z   - 3-space coordinates of vertex
    i,j,k   - 3 components of vertex normal
    s0, t0  - texture 0 coordinates
    r,g,b,a - static lighting color values
    s1, t1  - texture 1 coordinates
    s2, t2  - texture 2 coordinates
  -------------------------------------------------------------------*/
typedef struct {
    float x, y, z;
    float i, j, k;
    float s0, t0;
    float r, g, b, a;
    float s1, t1;
    float s2, t2;
} AtrVertex;

AtrVertex *atrVertexAllocate( FxU32 num );
void      atrVertexDeallocate( AtrVertex v[] );
void      atrVertexDefault( AtrVertex *v );
FxBool    atrVertexStore( AtrVertex *v, FILE *stream );
FxBool    atrVertexLoad( AtrVertex *v, FILE *stream );
void      atrVertexAssign( AtrVertex *dest, AtrVertex *src );
AtrVertex *atrVertexClone( AtrVertex *src );
void      atrVertexPrint( AtrVertex *v, FILE *stream, FxU32 indent );

typedef void (AtrVertexFunc)( AtrDstVertex *dest, 
                              AtrVertex *src,
                              AtrVertex *end );

/*
**------------------------------------------------------------------------
** TRANSFORM
**------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
  Structure: AtrXform
  Date: 3/18/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Matrix used to describe transformation among coordinate systems
    in the 3D environment.
  Members:
    data - Raw 4x4 Matrix Data
  -------------------------------------------------------------------*/
typedef struct {
    AtmMatrix4x4 data;
} AtrXform;

AtrXform *atrXformAllocate( FxU32 num );
void     atrXformDeallocate( AtrXform t[] );
void     atrXformDefault( AtrXform *t );
FxBool   atrXformStore( AtrXform *t, FILE *stream );
FxBool   atrXformLoad( AtrXform *t, FILE *stream );
void     atrXformAssign( AtrXform *dest, AtrXform *src );
AtrXform *atrXformClone( AtrXform *src );
void     atrXformPrint( AtrXform *t,
                        FILE     *stream, 
                        FxU32    indent );

void atrXformSetIdentity( AtrXform *t );
void atrXformInvert( AtrXform *dest, AtrXform *src );
void atrXformCat( AtrXform *dest, 
                  AtrXform *left, 
                  AtrXform *right );
void atrXformSetXRotation( AtrXform *t, float radians );
void atrXformSetYRotation( AtrXform *t, float radians );
void atrXformSetZRotation( AtrXform *t, float radians );
void atrXformSetTranslation( AtrXform *t, float x, float y, float z );
void atrXformSetRotation( AtrXform *t, AtmVector3 v, float radians );
void atrXformReturnToOrthonormal( AtrXform *dest, AtrXform *src );
void atrXformVectorMult( AtmVector3 dest, 
                         AtmVector3 src, 
                         AtrXform *t );
void atrXformPointAt( AtrXform *t, AtmVector3 target, 
                      AtmVector3 source, AtmVector3 up );

/*
**------------------------------------------------------------------------
** CAMERA
**------------------------------------------------------------------------
*/

typedef FxU32 AtrCameraFlags;
#define ATR_CAMERA_PERSPECTIVE   0x0
#define ATR_CAMERA_ORTHOGRAPHIC  0x1
#define ATR_CAMERA_FOVX          0x0
#define ATR_CAMERA_FOVY          0x2

/*-------------------------------------------------------------------
  Structure: AtrCamera
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Class describes a the properties of a particular viewer. The 
    default camera is a perspective camera with a 60 degree field
    of view in the X direction.
  Members:
    flags - flags describing type of camera
            ATR_CAMERA_PERSPECTIVE  - rendered using perspective proj
            ATR_CAMERA_ORTHOGRAPHIC - rendered using orthographic proj
            ATR_CAMERA_FOVX         - field of view is specified for X dir
            ATR_CAMERA_FOVY         - field of view is specified for Y dir
    fovRadians  - field of view angle in radians
    aspectRatio - ratio of width to height of viewport
    nearClip    - distance to near clipping plane
    farClip     - distance to far clipping plane
    xOffset     - xShear angle
    yOffset     - yShear angle
    lcsToWCS    - local coordinate system to world coordinate system 
                  transform, describes position and orientation of camera
  -------------------------------------------------------------------*/
typedef struct { 
    AtrCameraFlags flags;
    float      fovRadians;
    float      aspectRatio;
    float      nearClip;
    float      farClip;
    float      xOffset;
    float      yOffset;
    AtrXform   lcsToWCS;

    /* fields below this line are for internal use only */

    AtrXform   wcsToCCS;
} AtrCamera;

AtrCamera *atrCameraAllocate( FxU32 num );
void       atrCameraDeallocate( AtrCamera c[] );
void       atrCameraDefault( AtrCamera *c );
FxBool     atrCameraStore( AtrCamera *c, FILE *stream );
FxBool     atrCameraLoad( AtrCamera *c, FILE *stream );
void       atrCameraPrint( AtrCamera *c,
                           FILE      *stream, 
                           FxU32     indent );
void       atrCameraAssign( AtrCamera *dest, AtrCamera *src );
AtrCamera  *atrCameraClone( AtrCamera *src );

/*
**------------------------------------------------------------------------
** CANVAS
**------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
  Structure: AtrCanvas
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Class describing the boundaries of a rectangular window in which
    to render graphics.  The window must fall within the extents of 
    the current display.
  Members:
    xMin, yMin - lower left hand corner of window  ( inclusive )
    xMax, yMax - upper right hand corner of window ( inclusive )
  -------------------------------------------------------------------*/
typedef struct {
    FxU32 xMin;
    FxU32 yMin;
    FxU32 xMax;
    FxU32 yMax;
} AtrCanvas;

AtrCanvas *atrCanvasAllocate( FxU32 num );
void      atrCanvasDeallocate( AtrCanvas *c );
void      atrCanvasDefault( AtrCanvas *c );
FxBool    atrCanvasStore( AtrCanvas *c, FILE *stream );
FxBool    atrCanvasLoad( AtrCanvas *c, FILE *stream );
void      atrCanvasPrint( AtrCanvas *c,
                          FILE      *stream,
                          FxU32     indent );
void atrCanvasAssign( AtrCanvas *dest, AtrCanvas *src );
AtrCanvas *atrCanvasClone( AtrCanvas *src );

/*
**------------------------------------------------------------------------
** Image Table
**------------------------------------------------------------------------
*/
typedef FxU32 AtrImgFormat;
#define ATR_IMGFMT_RGB_332   0x00
#define ATR_IMGFMT_YIQ_422   0x01
#define ATR_IMGFMT_A_8       0x02
#define ATR_IMGFMT_I_8       0x03
#define ATR_IMGFMT_AI_44     0x04
#define ATR_IMGFMT_P_8       0x05
#define ATR_IMGFMT_ARGB_8332 0x08
#define ATR_IMGFMT_16_BIT    ATR_IMGFMT_ARGB_8332
#define ATR_IMGFMT_AYIQ_8422 0x09
#define ATR_IMGFMT_RGB_565   0x0a
#define ATR_IMGFMT_ARGB_1555 0x0b
#define ATR_IMGFMT_ARGB_4444 0x0c
#define ATR_IMGFMT_AI_88     0x0d
#define ATR_IMGFMT_AP_88     0x0e

#define ATR_IMGFMT_ARGB_8888 0x10
#define ATR_IMGFMT_RGB_888   0x11
#define ATR_IMGFMT_32_BIT    ATR_IMGFMT_ARGB_8888

#define ATR_IMGFMT_NONE ((AtrImgFormat)~0)

#define ATR_IMGFMT_ALPHA_8            ATR_IMGFMT_A_8
#define ATR_IMGFMT_INTENSITY_8        ATR_IMGFMT_I_8
#define ATR_IMGFMT_ALPHA_INTENSITY_44 ATR_IMGFMT_AI_44
#define ATR_IMGFMT_ALPHA_INTENSITY_88 ATR_IMGFMT_AI_88

typedef FxU32 AtrPalette[256];

typedef struct {
    FxU8  yRGB[16];
    FxI16 iRGB[4][3];
    FxI16 qRGB[4][3];
    FxU32 packedData[12];
} AtrNCCTable;

typedef union {
    AtrPalette  palette;
    AtrNCCTable nccTable;
} AtrImgTable;

/*-------------------------------------------------------------------
  Structure: AtrImg
  Date: 7/9
  Implementor(s): jdt, mlwp
  Library: AT Render
  Description: 
  Structure for storing image data. The data field holds a device 
  independent representation of the image. The devPrivate field holds
  the same information but in format that is supported and optimized
  by the driver. For example the D3D driver uses the devPrivate field
  to hold a pointer to the surface. In future Glide could use this 
  field to hold a 16 bit version of the image and the data field could
  be a 32 bit version. The data field is used when moving between 
  different devices and is saved/restored in the BOF file format. Once
  a driver has realized its own version of an image the device 
  independent version can be optionally freed to reclaim the space.
  Members:
  format     - valid image format
  width      - width of image in pixels
  height     - height of image in pixels 
  nLevels    - number of levels if mipmapped, 1 if not mipmapped
  name       - storage for a pointer to a name for image, file
               name of .3df file if image created from 3df
  data       - raw image data, upper left pixel to bottom right in
               row order
  table      - pointer to AtrImgTable union for paletized/compressed
               images
  devPrivate - drivers representation of the image (e.g. on D3D this is a 
               pointer to the surface)
  -------------------------------------------------------------------*/
typedef struct {
    AtrImgFormat format;
    FxU32        width;
    FxU32        height;
    FxU32        nLevels;
    char         *name;
    void         *data;
    AtrImgTable  *table;
    void         *devPrivate;
} AtrImg;

AtrImg *atrImgAllocate( FxU32 num );
void   atrImgDeallocate( AtrImg *i );
void   atrImgDefault( AtrImg *i );
FxBool atrImgStore( const AtrImg *i, FILE *stream );
FxBool atrImgLoad( AtrImg *i, FILE *stream );
FxBool atrImgCreateFromFile( AtrImg *i, const char *filename );
AtrImg *atrImgClone( const AtrImg *src );
void   atrImgAssign( AtrImg *dst, const AtrImg *src );
void   atrImgPrint( const AtrImg *i,
                    FILE   *stream, 
                    FxU32  indent );
void atrRenderImg( AtrImg *i, 
                   FxU32   screenX,
                   FxU32   screenY );

void atrRenderImgBuffer( AtrImg *i, 
                   FxU32   screenX,
                   FxU32   screenY, 
				   FxU32   buffer);

void atrGrabImg( AtrImg *i,
                 FxU32  screenX,
                 FxU32  screenY,
                 AtrBuffer buf );

/* for backwards compatibility */

#define atrImgSetPath( _path ) atuSetLoadPath(_path)
#define atrImgCreateFrom3df( _i, _filename ) atrImgCreateFromFile( _i, _filename )

/*
**------------------------------------------------------------------------
** LIGHT
**------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
  Structure: AtrLight
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Class describing the properties of light.      
  Members:
    flags - constant defining type of light

            ATR_LIGHT_AMBIENT - ambient light with no direction/attenuation
            ATR_LIGHT_DIRECTED - infinited directed light, no attenuation
            ATR_LIGHT_POSITIONAL - light source which emanates equally in 
                                   all directions from a point in space
            ATR_LIGHT_ATTENUATED - like a positional light source, but
                                   light energy is attenuated over distance
            *ATR_LIGHT_PROJECTOR - projects a texture
            

    color - color of light
    attenuation - 3 floats describing attenuation properties assuming
                  attenuation has been enabled.  Array Offsets Are
                  ATR_LIGHT_QUADRATIC - Quadratic attenuation constant
                  ATR_LIGHT_LINEAR    - linear attenuation constant
                  ATR_LIGHT_CONSTANT  - constant attenuation constant
    lcsToWCS - tranform describing position and orientation of light
  -------------------------------------------------------------------*/
typedef FxU32 AtrLightFlags; 
#define ATR_LIGHT_TYPE_MASK      0x0000000F
#define ATR_LIGHT_AMBIENT        0x00000000
#define ATR_LIGHT_DIRECTED       0x00000001
#define ATR_LIGHT_POSITIONAL     0x00000002
#define ATR_LIGHT_ATTENUATED     0x00000003
#define ATR_LIGHT_PROJECTOR      0x00000004

#define ATR_LIGHT_QUADRATIC      0
#define ATR_LIGHT_LINEAR         1
#define ATR_LIGHT_CONSTANT       2

typedef struct {
    AtrLightFlags flags;
    AtrColor      color;
    float         attenuation[3];
    struct {
        float    nearClip;
        float    fovInX;
        float    fovInY;
        FxU32    tag;
    } project;
    AtrXform      lcsToWCS;
} AtrLight;

AtrLight *atrLightAllocate( FxU32 num );             
void     atrLightDeallocate( AtrLight l[] );
void     atrLightDefault( AtrLight *l );
FxBool   atrLightStore( AtrLight *l, FILE *stream );
FxBool   atrLightLoad( AtrLight *l, FILE *stream );
AtrLight *atrLightClone( AtrLight *src );
void     atrLightAssign( AtrLight *dest, AtrLight *src );
void     atrLightPrint( AtrLight *l,
                        FILE     *stream, 
                        FxU32    indent );

/*
**------------------------------------------------------------------------
** Environment
**------------------------------------------------------------------------
*/

typedef FxU32 AtrEnvFlags;
#define ATR_FOG_MASK            0x0F
#define ATR_FOG_DISABLE         0x00
#define ATR_FOG_ON_DEPTH        0x01
#define ATR_FOG_ON_DEPTH_MP     0x02
#define ATR_HSR_MASK            0xF0
#define ATR_HSR_NONE            0x00
#define ATR_HSR_WBUFFER         0x10
#define ATR_FOG_TABLE_UPDATE   0x100

typedef FxU32 AtrEnvFogFunction;
#define ATR_FOGFUNC_LINEAR 0x0
#define ATR_FOGFUNC_EXP    0x1
#define ATR_FOGFUNC_EXP2   0x2

/*-------------------------------------------------------------------
  Structure: AtrFogTable
  Date: 3/21/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Table that implements fogging
  Members:
    data - packed fog table data
  -------------------------------------------------------------------*/
typedef struct {
    FxU8 data[64];
} AtrFogTable;

/*-------------------------------------------------------------------
  Structure: AtrEnv
  Date: 3/21/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Class describes "environmental" renderer effects.
  Members:
    flags - flags describing environment properties
            ATR_FOG_DISABLE  - turn off fogging
            ATR_FOG_ON_DEPTH - fog on w-depth from viewer
            ATR_HSR_NONE     - no hidden surface removal
            ATR_HSR_WBUFFER  - w-buffer hidden surface removal 
    fogColor - color blended in by fog unit
    fogTable - table describing fogging function
  -------------------------------------------------------------------*/
typedef struct {
    AtrEnvFlags       flags;
    AtrColor          fogColor;
    AtrFogTable       fogTable;
    AtrEnvFogFunction fogFunc;
    float             fogDensity;
    float             fogNearW;
    float             fogFarW;
} AtrEnv;

AtrEnv *atrEnvAllocate( FxU32 num );
void   atrEnvDeallocate( AtrEnv e[] );
void   atrEnvDefault( AtrEnv *e );
FxBool atrEnvStore( AtrEnv *e, FILE *stream );
FxBool atrEnvLoad( AtrEnv *e, FILE *stream );
AtrEnv *atrEnvClone( AtrEnv *src );
void   atrEnvAssign( AtrEnv *dest, AtrEnv *src );
void   atrEnvPrint( AtrEnv *e,
                    FILE     *stream, 
                    FxU32    indent );

void   atrEnvFogTable( AtrEnv            *e,
                       AtrEnvFogFunction func,
                       float             density,
                       float             nearW,
                       float             farW );

/*
**------------------------------------------------------------------------
** Textures
**------------------------------------------------------------------------
*/

typedef FxU32 AtrTexelFx;
#define ATR_TEXELFX_0    0x0
#define ATR_TEXELFX_1    0x1
#define ATR_TEXELFX_BOTH 0x2


/*-------------------------------------------------------------------
  Structure: AtrTexInfo
  Date: 7/1
  Implementor(s): jdt
  Library: AT Render
  Description:
  Contains information associated with a particular
  texture handles
  Members:
  tmu - which texel fx unit memory pool the texture is destined
  for.  Valid values are:
        ATR_TEXELFX_0   - downstream texture mapping unit
        ATR_TEXELFX_1   - upstream texture mapping unit ( Amethyst )
        ATR_TEXELFX_BOTH - even levels on 0 odd on 1 ( Amethyst )
  sizeInBytes - amount of texture ram consumed
  img - pointer to image data if any, NULL if no image associated
  -------------------------------------------------------------------*/
typedef struct {
    AtrTexelFx   tmu;
    FxU32        sizeInBytes;
    AtrImg       *img;
} AtrTexInfo;

typedef void *AtrTexHandle;
#define ATR_NULL_TEXHANDLE 0

/* TBD: this was private to rtex.c should be moved to a private header file */

#define ATR_TEXENT_FREE      0   /* texture entry is in free list */
#define ATR_TEXENT_ALLOCATED 1   /* texture entry is associated with an image */
#define ATR_TEXENT_CACHED    2   /* texture entry is in texture cache */

typedef struct _ATRTE {
    FxU32         state;
    AtrTexInfo    atrInfo;
    AtrTexHandle  other;
    FxBool        dirty;
    struct _ATRTE *next;
    struct _ATRTE *prev;
} _AtrTexEntry;

void atrSetTexPlanarOffset( float s, float t);
AtrTexHandle atrTexNewHandle( AtrTexelFx tmu );
void         atrTexDeleteHandle( AtrTexHandle handle );
FxBool       atrTexAssociate( AtrTexHandle handle, 
                              AtrImg       *img );
void         atrTexSource( AtrTexHandle handle );

void atrTexDirty( AtrTexHandle handle );
void atrTexInfo( AtrTexHandle handle, AtrTexInfo *info );
void atrTexMemFlush( FxU32 tmu );

/*
**------------------------------------------------------------------------
** Material
**------------------------------------------------------------------------
*/

#define ATR_TEX0SHIFT             0
#define ATR_TEX1SHIFT             8

typedef FxU32 AtrTexMMMode;
#define ATR_TEXMIPMAP_DISABLE        0x0
#define ATR_TEXMIPMAP_NEAREST        0x1
#define ATR_TEXMIPMAP_NEAREST_DITHER 0x2

#define ATR_NUM_MMODES 3

typedef FxU32 AtrTexClamp;
#define ATR_TEXCLAMP_WRAP         0x0
#define ATR_TEXCLAMP_CLAMP        0x1

typedef FxU32 AtrTexFilter;
#define ATR_TEXFILTER_POINT_SAMPLED 0x0
#define ATR_TEXFILTER_BILINEAR      0x1

#define ATR_NUM_FILTER_MODES 2

typedef FxU32 AtrCompareFnc;
#define ATR_CMP_NEVER    0x0
#define ATR_CMP_LESS     0x1
#define ATR_CMP_EQUAL    0x2
#define ATR_CMP_LEQUAL   0x3
#define ATR_CMP_GREATER  0x4
#define ATR_CMP_NOTEQUAL 0x5
#define ATR_CMP_GEQUAL   0x6
#define ATR_CMP_ALWAYS   0x7

typedef FxU32 AtrIRGBSrc;
#define ATR_IRGBSRC_MASK          0x000f
#define ATR_IRGBSRC_NONE          0x0000
#define ATR_IRGBSRC_LIGHTING      0x0001
#define ATR_IRGBSRC_STATIC        0x0002

typedef FxU32 AtrIASrc;
#define ATR_IASRC_MASK            0x000C
#define ATR_IASRC_NONE            0x0000
#define ATR_IASRC_STATIC          0x0004

typedef FxU32 AtrCRGBSrc;
#define ATR_CRGBSRC_MASK          0x0030
#define ATR_CRGBSRC_NONE          0x0000
#define ATR_CRGBSRC_LIGHTING      0x0010
#define ATR_CRGBSRC_STATIC        0x0020

typedef FxU32 AtrTexSrc;
#define ATR_TEXSRC_MASK           0x0f00
#define ATR_TEXSRC_NONE           0x0000
#define ATR_TEXSRC_DECAL          0x0100
#define ATR_TEXSRC_DETAIL         0x0200
#define ATR_TEXSRC_EMAP           0x0300
#define ATR_TEXSRC_LMAP           0x0400
#define ATR_TEXSRC_PROJECTED      0x0500

#define ATR_TEXSRC0_MASK          (ATR_TEXSRC_MASK<<ATR_TEX0SHIFT)
#define ATR_TEXSRC1_MASK          (ATR_TEXSRC_MASK<<ATR_TEX1SHIFT)

typedef FxU32 AtrTCSrc;
#define ATR_TCSRC_MASK            0xf000
#define ATR_TCSRC_NONE            0x0000
#define ATR_TCSRC_TC0             0x1000
#define ATR_TCSRC_TC1             0x2000
#define ATR_TCSRC_TC2             0x3000
#define ATR_TCSRC_EMAP            0x4000
#define ATR_TCSRC_LMAP            0x5000
#define ATR_TCSRC_PROJECTED       0x6000
#define ATR_TCSRC_PLANAR          0x7000
// Denis. Fixing detail texture
#define ATR_TCSRC_TC0_SRC1_SCALE_TC0 0x8000
#define ATR_TCSRC_TC0_SCALE		  0x9000
#define ATR_TCSRC_TC1_SCALE		  0xA000

#define ATR_TCSRC0_MASK           (ATR_TCSRC_MASK<<ATR_TEX0SHIFT)
#define ATR_TCSRC1_MASK           (ATR_TCSRC_MASK<<ATR_TEX1SHIFT)

typedef FxU32 AtrMatFlags;
#define ATR_MAT_TEX_MASK                0x00FF
#define ATR_MAT_TEX_NONE                0x0000
#define ATR_MAT_TEX_DECAL               0x0001
#define ATR_MAT_TEX_DECAL0              ATR_MAT_TEX_DECAL
#define ATR_MAT_TEX_DECAL1              0x0002
#define ATR_MAT_TEX_LODBLEND_PASS0      0x0003
#define ATR_MAT_TEX_LODBLEND_PASS1      0x0004
#define ATR_MAT_TEX_LODBLEND_SINGLEPASS 0x0005
#define ATR_MAT_TEX_DETAIL_PASS0        0x0006
#define ATR_MAT_TEX_DETAIL_PASS1        0x0007
#define ATR_MAT_TEX_DETAIL_SINGLEPASS   0x0008
#define ATR_MAT_TEX_EMAP                0x0009
#define ATR_MAT_TEX_LMAP                0x000A
#define ATR_MAT_TEX_PROJECTED           0x000B
#define ATR_MAT_TEX_DECAL_X_PROJECTED   0x000C
#define ATR_MAT_TEX_PLANAR              0x000D
#define ATR_MAT_TEX_DECAL_X_PLANAR      0x000E
#define ATR_MAT_TEX_DECAL_ADD_PLANAR    0x000F
#define ATR_MAT_TEX_PLANAR_1            0x0010

#define ATR_MAT_LIGHTING_MASK              0x3000
#define ATR_MAT_LIGHTING_NONE              0x0000
#define ATR_MAT_LIGHTING_MULTIPLY          0x1000
#define ATR_MAT_LIGHTING_ADD               0x2000
#define ATR_MAT_LIGHTING_BLEND_ON_TEXALPHA 0x3000

#define ATR_MAT_LIGHTSRC_MASK           0xC000
#define ATR_MAT_LIGHTSRC_NONE           0x0000
#define ATR_MAT_LIGHTSRC_LIGHT          0x4000
#define ATR_MAT_LIGHTSRC_STATIC         0x8000
#define ATR_MAT_LIGHTSRC_CONSTANT       0xC000

#define ATR_MAT_FB_MASK                 0x0F00
#define ATR_MAT_FB_ASSIGN               0x0000
#define ATR_MAT_FB_MULTIPLY             0x0100
#define ATR_MAT_FB_ADD                  0x0200
#define ATR_MAT_FB_BLEND                0x0300
#define ATR_MAT_FB_ZERO                 0x0400
#define ATR_MAT_FB_MULT_MP_FOG          0x0500

#define ATR_MAT_EMAP              (ATR_MAT_TEX_EMAP|ATR_MAT_LIGHTING_NONE|ATR_MAT_FB_ASSIGN)
#define ATR_MAT_LMAP              (ATR_MAT_TEX_LMAP|ATR_MAT_LIGHTING_NONE|ATR_MAT_FB_ASSIGN)
#define ATR_MAT_DECAL             (ATR_MAT_TEX_DECAL|ATR_MAT_LIGHTING_NONE|ATR_MAT_FB_ASSIGN)
#define ATR_MAT_DECAL1            (ATR_MAT_TEX_DECAL1|ATR_MAT_LIGHTING_NONE|ATR_MAT_FB_ASSIGN)
#define ATR_MAT_DECAL_X_LIGHTING  (ATR_MAT_TEX_DECAL|ATR_MAT_LIGHTSRC_LIGHT\
                                   |ATR_MAT_LIGHTING_MULTIPLY)
#define ATR_MAT_DECAL_X_LIGHTING1  (ATR_MAT_TEX_DECAL1|ATR_MAT_LIGHTSRC_LIGHT\
                                   |ATR_MAT_LIGHTING_MULTIPLY)
#define ATR_MAT_EMAP_X_LIGHTING   (ATR_MAT_TEX_EMAP|ATR_MAT_LIGHTSRC_LIGHT\
                                   |ATR_MAT_LIGHTING_MULTIPLY)
#define ATR_MAT_GSHADE            (ATR_MAT_LIGHTSRC_LIGHT|\
                                   ATR_MAT_LIGHTING_ADD)

typedef struct { 
    FxI32 bias; 
    FxU32 scale; 
    float max; 
} AtrTexDetail;

/* Denis. Adding more control for detail texturing */
void atrSetTCScaleX( FxU32 tmu, float sx );
void atrSetTCScaleY( FxU32 tmu, float sy );
float atrGetTCScaleX( FxU32 tmu );
float atrGetTCScaleY( FxU32 tmu );


/*-------------------------------------------------------------------
  Structure: AtrMaterial
  Date: 3/21/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Class describes surface material property information for lighting
    equations texturing processes.  Also defines how data will be piped
    to hardware from ATR and how hardware will manage that data 
    internally.  
  Members:
  
    typeflag - type specified in atrMaterialSetup

    *** Surface Properties ***
    
    emissive - emissive color of surface, this is color that is
               added independant of any lighting
    diffuse - diffuse color of surface, this is color that is multiplied
              by the color derived from the diffuse lighting computation
    specular - specular color of surface, this is color that is 
               multiplied by the color derived form the specular lighting
               computation
    specExponent - also called shininess exponent.  Determines rate of
                   saturation of specular lighting equation.
                   Values between 1 and 40 are typical
    constant - constant color for CRGBSRC
    chromaKeyEnable - FXTRUE if chromakey enabled
    chromakeyValue chromakey value
    texture - pointer to texture(s)
    projectedTag - tag that matches a material pass to a 
                   texture projector
    isTwoSided - bool telling whether to backface cull or not.
    lmapCoords - array of texture coordinates for all lighting maps
    successor - pointer to next texture if chaining, 0 otherwise

    *** ATR Routing ***

    irgbSrc - source for iterated rgb data
              ATR_IRGBSRC_NONE     - no itrgb
              ATR_IRGBSRC_LIGHTING - compute lighting equation
              ATR_IRGBSRC_STATIC   - copy from vertex color
    iaSrc -   source for iterated rgba data
              ATR_IASRC_NONE      - no itasrc
              ATR_IASRC_STATIC    - copy from vertex color
    crgbSrc - source for constant rgba data
              ATR_CRGBSRC_NONE - no crgba
              ATR_CRGBSRC_LIGHTING - per polygon lighting equation
              ATR_CRGBSRC_STATIC - constant color from material
    texSrc - source for texture data
              ATR_TEXSRC_NONE - no texture
              ATR_TEXSRC_DECAL- decal texture in material texture pointer
              ATR_TEXSRC_DETAIL - detail texture in material texture pointer
              ATR_TEXSRC_EMAP - env map in material texture pointer
              ATR_TEXSRC_LMAP - lighting map in model lighting map pointer
    tcSrc - source for texture coordinate data
              ATR_TCSRC_NONE - unspecified texture coordinates
              ATR_TCSRC_TC0 - texture coordinate 0 in vertex struct
              ATR_TCSRC_TC1 - texture coordinate 1 in vertex struct
              ATR_TCSRC_TC2 - texture coordinate 2 in vertex struct
              ATR_TCSRC_EMAP - environment mapping computation
              ATR_TCSRC_LMAP - lighting map texture coordinates
              ATR_TCSRC_PROJECTED - projected texture computation

    *** Hardware Routing ***

    atestFunc - alpha testing function ( ATR_CMP_ALWAYS to disable )
    aReference - reference alpha value
    texCFunction - texture combine rgb function for each tmu ( see glide ref )
    texCFactor - texture combine rgb factor for each tmu ( see glide ref )
    texAFunction - texture combine rgb function for each tmu ( see glide ref )
    texAFactor - texture combine rgb factor for each tmu ( see glide ref )
    texCInvert - texture combine rgb-Invert for each tmu ( see glide ref )
    texAInvert - texture combine a-Invert for each tmu ( see glide ref )
    texLODBlend - enable Lod blending
    texDetail   - parameters that control detail texturing
                  bias  - where blending begins [-32,+32]
                  scale - slope of blendign [0,7]
                  max   - maximum blending  [0.0f,1.0f]
    ccuFunction - color combine function ( see glide ref )
    ccuFactor - color combine factor ( see glide ref )
    ccuLocal - color combine local input ( see glide ref )
    ccuOther - color combine other input ( see glide ref )
    ccuInvert - color combine invert flag ( see glide ref )
    acuFunction - alpha combine function ( see glide ref )
    acuFactor - alpha combine factor ( see glide ref )
    acuLocal - alpha combine local input ( see glide ref )
    acuOther - alpha combine other input ( see glide ref )
    acuInvert - alpha combine invert flag ( see glide ref )
    abuSrcFactor - alpha blender rgb src factor ( see glide ref )
    abuDstFactor - alpha blender rgb dst factor ( see glide ref )
    depthMask - turn writes to the depth buffer on and off ( 0/1 = off/on )
  -------------------------------------------------------------------*/

typedef struct _atrmat {
    FxU32    typeFlag;  /* as per atrMaterialSetup */
    /* Lighting Properties */
    AtrColor emissive;
    AtrColor diffuse;
    AtrColor specular;
    FxU32    specExponent;

    /* CCRGB */
    FxU32 constant;

    /* Chromakey stuff */
    FxBool chromaKeyEnable;
    FxU32  chromaKeyValue;
    
    /* Textures */    
    AtrTexHandle texture[2];
    FxU32        projectedTag;
    float        planarScale;

    /* Culling and TwoSided Lighting */
    FxBool      isTwoSided;

    AtmVector2  lmCoords[3];

    struct _atrmat *successor;

    /* ATR Routing */
    AtrIRGBSrc irgbSrc;
    AtrIASrc   iaSrc;
    AtrCRGBSrc crgbSrc;
    AtrTexSrc  texSrc[2];
    AtrTCSrc   tcSrc[2];

    /* TCU Routing */
    FxU32          tcuCFunction[2];
    FxU32          tcuCFactor[2];
    FxU32          tcuAFunction[2];
    FxU32          tcuAFactor[2];
    FxU32          tcuCInvert[2];
    FxU32          tcuAInvert[2];

    /* Alpha Compare Func Properties */
    AtrCompareFnc atestFunc;
    FxU8          aReference;

    /* Texture Properties */
    AtrTexClamp    texSClamp[2];
    AtrTexClamp    texTClamp[2];
    AtrTexMMMode   texMMMode[2];
    AtrTexFilter   texMinFilter[2];
    AtrTexFilter   texMagFilter[2];
    FxBool         texLODBlend[2];
    AtrTexDetail   texDetail[2];

    /* CCU Routing */
    FxU32 ccuFunction;
    FxU32 ccuFactor;
    FxU32 ccuLocal;
    FxU32 ccuOther;
    FxU32 ccuInvert;

    /* ACU Routing */
    FxU32 acuFunction;
    FxU32 acuFactor;
    FxU32 acuLocal;
    FxU32 acuOther;
    FxU32 acuInvert;

    /* ABU Routing */
    FxU32 abuSrcFactor;
    FxU32 abuDstFactor;
    FxU32 depthMask;

    FxU32 sysFlags;

    FxU32 texture_combine[2];
    FxU32 color_combine;
    FxU32 alpha_combine;
    FxU32 alpha_blend;
} AtrMaterial;

#define TC_MODE_INVALID    0
#define TC_MODE_1          1
#define TC_MODE_2          2
#define TC_MODE_3          3
#define TC_MODE_4          4
#define TC_MODE_5          5
#define TC_MODE_6          6
#define TC_MODE_7          7
#define TC_MODE_8          8
#define TC_MODE_9          9
#define TC_MODE_10         10
#define TC_MODE_11         11
#define TC_MODE_12         12

#define CC_LOCAL_INVALID   0
#define CC_LOCAL_NONE      1
#define CC_LOCAL_ITERATED  2
#define CC_LOCAL_CONSTANT  3

#define CC_MODE_INVALID    (0<<16)
#define CC_MODE_1          (1<<16)
#define CC_MODE_2          (2<<16)
#define CC_MODE_3          (3<<16)
#define CC_MODE_4          (4<<16)
#define CC_MODE_5          (5<<16)

#define AC_MODE_INVALID    0
#define AC_MODE_1          1

#define AB_MODE_INVALID    0
#define AB_MODE_1          1
#define AB_MODE_2          2
#define AB_MODE_3          3
#define AB_MODE_4          4
#define AB_MODE_5          5
#define AB_MODE_6          6

void        atrMaterialModify(AtrMaterial *m);
AtrMaterial *atrMaterialAllocate( FxU32 num );
void        atrMaterialDeallocate( AtrMaterial *m );
void        atrMaterialDefault( AtrMaterial *m );
FxBool      atrMaterialStore( AtrMaterial *m, FILE *stream );
FxBool      atrMaterialLoad( AtrMaterial *m, FILE *stream );
void        atrMaterialAssign( AtrMaterial *dest, AtrMaterial *src );
AtrMaterial *atrMaterialClone( AtrMaterial *src );
void        atrMaterialPrint( AtrMaterial *m,
                              FILE        *stream, 
                              FxU32       indent );

void atrMaterialSetup( AtrMaterial *m, FxU32 typeFlag );

/*
**------------------------------------------------------------------------
** Triangles
**------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
  Structure: _AtrTriSetNode
  Date: 3/21/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    A chunk in a tri-set model.  Contains a grouping
    of ATR_CHUNK_SIZE or less vertices.
  Members:
    numVertices - actual number of vertices
    connectivity - triangles defined as indices packed into low
                   order bytes of a long.  *connectivity == 0x0
                   defines end of connectivity list.
    lightMaps    - pointer to lightmaps in same order as triangles in
                   connectivity list
    next        - pointer to next TriSetNode, 0 means end of list
  -------------------------------------------------------------------*/
typedef struct _atrtsn {
    FxU32          numVertices;
    AtrVertex      *vertices;
    FxU32          *connectivity;
    AtrTexHandle   *lightMaps;
    struct _atrtsn *next;
} _AtrTriSetNode;

/*-------------------------------------------------------------------
  Structure: AtrTriSet
  Date: 3/22/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    A set of triangles all of whom will be rendered in one material and
    all of whom are fixed in space relative to one another.  TriSets
    are meant to be created automatically using the atrTriSetBegin(),
    atrTriSetVertex(), atrTriSetEnd() apis, they cannot be created
    manually by the application.
  Members:
    nodes - nodes that contain actual vertices, connectivity
    bbMin,... - bounding box extents
    texturePosition - current offset for scrolling textures
  -------------------------------------------------------------------*/
typedef struct {
    _AtrTriSetNode *nodes;
    float      bbMinX;
    float      bbMinY;
    float      bbMaxX;
    float      bbMaxY;
    AtmVector2 texturePosition;
} AtrTriSet;

AtrTriSet *atrTriSetAllocate( FxU32 num );
void      atrTriSetDeallocate( AtrTriSet *t );
void      atrTriSetDefault( AtrTriSet *t );
FxBool    atrTriSetStore( AtrTriSet *t, FILE *stream );
FxBool    atrTriSetLoad( AtrTriSet *t, FILE *stream );
#ifdef NOTDEFINED
void      atrTriSetAssign( AtrTriSet *dest, AtrTriSet *src );
AtrTriSet *atrTriSetClone( AtrTriSet *src );
void        atrMaterialPrint( AtrMaterial *m,
                              FILE        *stream, 
                              FxU32       indent );
#endif

void      atrTriSetBegin( AtrTriSet *t );
void      atrTriSetVertex( AtrTriSet *t, 
                           AtrVertex *v );
void      atrTriSetLightMap( AtrTriSet   *t,
                             AtrTexHandle m );
void      atrTriSetCalcNormals( void );
void      atrTriSetSurfaceNormal( AtrTriSet *t, AtmVector3 n );
void      atrTriSetEnd( AtrTriSet *t );


/*
**------------------------------------------------------------------------
** OpenTriSet
**------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
  Structure: AtrOpenTriSet
  Date: 6/14
  Implementor(s): jdt
  Library: AT Render
  Description:
    Container for a list of AtrVertices, AtrDestVertices,
    a connectivity list,and a list of texture coordinates.
  Members:
    numVertices  -  number of vertices in srcVertices and dstVertices
    srcVertices  -  object space vertex data for object
    dstVertices  -  storage for screen space vertex data
    numTriangles -  number of triangles in the connectivity list
    connectivity -  clockwise ordered indices into srcVertices which
                    define triangle connectivity in the object
    normals      -  list of surface normals used for flat shading
                    and potentially for culling
    texCoords    -  per-triangle texture coordinates must be same
                    length as connectivity list ( optional ) if 
                    this texCoords is null, then the per-vertex
                    texture coordinates are used in rendering
  -------------------------------------------------------------------*/
typedef struct {
    FxU32         numVertices;
    AtrVertex     *srcVertices;
    AtrDstVertex  *dstVertices;
    FxU32         numTriangles;
    FxU32         (*connectivity)[3];
    AtmVector3    *normals;
    AtmVector2    (*texCoords)[3];
} AtrOpenTriSet;

AtrOpenTriSet *atrOpenTriSetAllocate( FxU32 num );
void          atrOpenTriSetDeallocate( AtrOpenTriSet *t );
void          atrOpenTriSetDefault( AtrOpenTriSet *t );
FxBool        atrOpenTriSetStore( AtrOpenTriSet *t, FILE *stream );
FxBool        atrOpenTriSetLoad( AtrOpenTriSet *t, FILE *stream );
void          atrOpenTriSetCalcNormals( AtrOpenTriSet *t);


#ifdef NOTDEFINED
void          atrOpenTriSetAssign( AtrOpenTriSet *dest, AtrOpenTriSet *src );
AtrOpenTriSet *atrOpenTriSetClone( AtrOpenTriSet *src );
void          atrOpenTriSetPrint( AtrOpenTriSet *t,
                                  FILE          *stream, 
                                  FxU32         indent );
#endif




/*
**------------------------------------------------------------------------
** Statistics
**------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
  Structure: AtrStats
  Date: 3/23/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Contains interesting statistical information for the whole library.
    Everything is reset on library reset and when atrStatsReset() is
    called explicitly.
  Members:
    frames - number of frames since atrStatsReset()
    highOrderTSC - high order bits of the time stamp counter
    lowOrderTSC - low order bits of the time stamp counter
    elapsedTime - elapsed time in seconds since atrStatsReset()
    totalTris - total triangles processed by library since reset
    clippedTris - clipped triangles since reset
    pixelsIn - sst-1 pixelsIn counter, number of pixels processed
    ffPixelIn - number of pixels rendered due to fast fills.
    pixelsOut - number of pixels actually rendered to screen
    textureDownloads - number of textures downloaded since reset
    peakTexMemoryInFrame - peak texmemory consumption in a single frame
    peakTexCacheMissesInFrame - peak cache misses in a single frame
  -------------------------------------------------------------------*/
typedef struct {
    FxU32 frames;
    FxU32 highTSC;
    FxU32 lowTSC;
    float elapsedTime;
    FxU32 totalTris;
    FxU32 totalVerts;
    FxU32 clippedTris;
    FxU32 pixelsIn;
    FxU32 ffPixelsIn;
    FxU32 pixelsOut;
    FxU32 textureDownloads[2];
    FxU32 peakTexMemoryInFrame[2];
    FxU32 peakTexCacheMissesInFrame[2];
    FxU32 texBytesUsed[2];
    FxU32 nccTableDownloads[2];
    FxU32 paletteDownloads[2];
} AtrStats;

void atrStatsReset();
void atrStatsRetrieve( AtrStats *s );

/*
**------------------------------------------------------------------------
** Library State/Query Functions
**------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
  Structure: AtrD3DDriverInfo
  Date: 10/09/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Contains information used to configure the driver. Which information
    is used is driver dependent. For example: D3D uses the hWnd to determine 
    the display resolution. Glide uses the width and height fields to do the
    same thing.
  Members:
    hWnd              -  handle to window to be used for rendering
    width             -  width of display
    height            -  height of the display
    emulation         -  Do not use hardware
    refreshRate       -  screen refresh rate
    numBuffers        -  single or double buffered
    smoothingMode     -  enables video interpolation
    fullScreen        -  access the display in full screen mode
    info              -  driver dependent
  -------------------------------------------------------------------*/

typedef struct {
    void   *hWnd;
    FxU32   width;
    FxU32   height;
    FxBool  emulation;
    int    refreshRate;
    int    numBuffers;
    int    smoothingMode;
    FxBool fullScreen;
    void  *info;
    char  *driverName;
} AtrDriverInfo;

typedef FxU32 AtrDitherMode;
#define ATR_DITHER_DISABLE       0x0
#define ATR_DITHER_ENABLE        0x1
#define ATR_DITHER_2x2           0x1
#define ATR_DITHER_4x4           0x2

void atrDitherMode(AtrDitherMode mode); 

typedef void *AtrContext;

AtrContext atrInit( char *driverName, AtrDriverInfo *driverInfo);

FxBool atrPause(FxBool flag);

FxBool atrIsDrawable( AtrContext ctx );

FxBool atrResize(AtrContext ctx, int w, int h);

FxBool atrMove(AtrContext ctx, int x, int y);

void   atrShutdown( void );

void atrRenderBuffer( AtrBuffer buf );

void atrQueryDriverCaps( AtrDriverCaps *caps );

void atrExit( void );
void atrEnter( void );

void atrPostCatPushXform( AtrXform *t );
void atrPreCatPushXform( AtrXform *t );
void atrPushXform( AtrXform *t );
void atrPopXform( FxBool update );
void atrQueryXform( AtrXform *t );

void atrSelectCamera( AtrCamera *c );
void atrQueryCamera( AtrCamera *c );

void atrSelectCanvas( AtrCanvas *c );
void atrQueryCanvas( AtrCanvas *c );

void  atrAddLight( AtrLight *l, FxU32 tag );
FxU32 atrRemoveLight( FxU32 tag );

void atrPushEnv( AtrEnv *e );
void atrPopEnv( FxBool update );
void atrQueryEnv( AtrEnv *e );

void atrPushMaterial( AtrMaterial *m );
void atrPopMaterial( FxBool update );
void atrQueryMaterial( AtrMaterial *m );

void atrQueryLCSEyePoint( AtmVector3 eyepoint );

#define ATR_WBUFFER_CLEAR 0xFFFF
#define ATR_WBUFFER_SET   0x0000

void atrClearCanvas( float r, float g, float b, FxU16 za );
void atrSwapBuffer( FxU32 sync );
FxBool atrBeginScene(FxU32 *viewWidth, FxU32 *viewHeight);
FxBool atrEndScene(void);
void atrRenderTriSet( AtrTriSet *t );
void atrRenderTri( AtrVertex *a, AtrVertex *b, AtrVertex *c );
void atrRenderSegment( AtrVertex *a, AtrVertex *b );

void atrRenderTriWF( AtrVertex *a, AtrVertex *b, AtrVertex *c );
void atrRenderTriSetWF( AtrTriSet *t );

void _atrDrawString( char *string, 
                 float char_width, float char_height,
                 float xmin, float ymin, FxBool opaque );

void atrDrawString( char *string, float char_width, float char_height, 
                    float xmin, float ymin ) ;

void atrDrawStringOpaque( char *string, float char_width, float char_height,
                          float xmin, float ymin );

/*---------------------------------------------------------
  Probationary Routines
  ---------------------------------------------------------*/
void atrTransformOpenTriSet( AtrOpenTriSet *t );
void atrApplyMaterialToOpenTriSet( AtrOpenTriSet *t );
void atrRenderOpenTriSet( AtrOpenTriSet *t );
void atrRenderOpenTriSetWF( AtrOpenTriSet *t );

void atrSetSpecialCallback( AtrVertexFunc *callback );
void atrSpecialRenderTriSet( AtrTriSet *t );

void atrSpecialTransformOpenTriSet( AtrOpenTriSet *t );

void atrRender2DTri( AtrVertex *a, AtrVertex *b, AtrVertex *c );

AtrTriSet *atrPrimSphere( float radius, int nParallels, int nMeridians );
AtrTriSet *atrPrimCube( float s );
AtrTriSet *atrPrimPlane( float s );
AtrTriSet *atrPrimArrow( float length );

/*---------------------------------------------------------
  Low level driver interface routines
  ---------------------------------------------------------*/

void atrDrawPoint( const AtrDstVertex *p );

void atrDrawLine( const AtrDstVertex *a, const  AtrDstVertex *b );

void atrDrawTriangle(const AtrDstVertex *a, const AtrDstVertex *b, 
                     const AtrDstVertex *c);

ENDFDECL()                

#endif



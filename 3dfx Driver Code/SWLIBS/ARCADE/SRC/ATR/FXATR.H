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
** $Date: 10/11/00 7:34:01 PM$ 
**
*/

#ifndef _FXATR_H_
#   define _FXATR_H_

/*
**------------------------------------------------------------------------
** TYPES
**------------------------------------------------------------------------
*/

#include <3dfx.h>
#include "atrender.h"
#define GLIDE_HARDWARE
#include <glide.h>

BEGINFDECL()

/*
**------------------------------------------------------------------------
** MACROS
**------------------------------------------------------------------------
*/
/* was 19 */
#define ATR_SNAP_BIAS ((float)(1L<<19)) 

extern float _atrSnapBias;
extern float _atrDriverXOffset;
extern float _atrDriverYOffset;

/*
**------------------------------------------------------------------------
** Internal Global Data/Constants
**------------------------------------------------------------------------
*/
#define ATR_MAX_LIGHTS 64

#define ATR_LIGHTFUNC_MASK     0x0f000000
#define ATR_LIGHTFUNC_SHIFT    24
#define ATR_LIGHTFUNC_DIFFUSE  0x01000000
#define ATR_LIGHTFUNC_SPECULAR 0x02000000

#define ATR_LT_ONE_SIDED  0
#define ATR_LT_TWO_SIDED  1
#define ATR_NUM_MAT_MODES     2

#define ATR_LT_AMBIENT        0
#define ATR_LT_DIRECTED       1
#define ATR_LT_POSITIONAL     2
#define ATR_LT_ATTENUATED     3
#define ATR_LT_PROJECTED      4
#define ATR_NUM_LIGHT_TYPES   5

#define ATR_LT_INIT        0
#define ATR_LT_ACCUM       1
#define ATR_NUM_ACCUM_MODES    2

#define ATR_LT_NO_CLAMP    0
#define ATR_LT_CLAMP       1
#define ATR_NUM_CLAMP_MODES    2

#define ATR_LT_DIFFUSE    0
#define ATR_LT_SPEC       1
#define ATR_LT_DIFFSPEC   2
#define ATR_LT_USER       3
#define ATR_NUM_MAT_TYPES     4


#define ATR_NULL_TEXADDR ((FxU32)~0)


/*
**------------------------------------------------------------------------
** Internal Data Definitions
**------------------------------------------------------------------------
*/


/* Rendering Callbacks  */
AtrVertexFunc _atrIRGBSRC_LIGHTING;
AtrVertexFunc _atrIRGBSRC_STATIC;
AtrVertexFunc _atrIASRC_STATIC;
AtrVertexFunc _atrTCSRC0_TC0;
AtrVertexFunc _atrTCSRC0_TC1;
AtrVertexFunc _atrTCSRC0_TC2;
AtrVertexFunc _atrTCSRC0_PROJECTED;
AtrVertexFunc _atrTCSRC0_PLANAR;
AtrVertexFunc _atrTCSRC0_EMAP;
AtrVertexFunc _atrTCSRC1_TC0;
AtrVertexFunc _atrTCSRC1_TC1;
AtrVertexFunc _atrTCSRC1_TC2;
AtrVertexFunc _atrTCSRC1_EMAP;
AtrVertexFunc _atrTCSRC1_PROJECTED;
AtrVertexFunc _atrTCSRC1_PLANAR;

AtrVertexFunc _atrTCSRC0_2DTC0;

AtrVertexFunc _atrTCSRC0_TC0_SRC1_SCALE_TC0;
AtrVertexFunc _atrTCSRC0_TC0_SCALE;
AtrVertexFunc _atrTCSRC1_TC1_SCALE;

/*-------------------------------------------------------------------
  Structure: _AtrRenderCache
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Structure containing aligned data for rendering
  Members:
    lcsToCCS - the current Local Coordinate Syste to Clip Coord System
               matrix
    xScale, xOffset - the current viewport scaling and offset constants
    yScale, yOffset - the current viewport scaling and offset constants
    cameraPositionInLCS - the current camera position in object space
    matBaseColor - the emissive color of the current material
                   plus the sum of the dif color * all ambient 
                   lights
    irgbSrcFunc - functions pointer for generating iterated rgb data
    texCoordSrcFunc - function pointer for generating texture coords
    nearClip, farClip - clipping plane distances
    destVertex - the rendering vertex cache
-------------------------------------------------------------------*/
typedef struct {
    AtrXform lcsToCCS;
    float xScale, xOffset, yScale, yOffset;
    float nearClip, farClip;
    AtrColor matBaseColor;
    AtmVector3 cameraPositionInLCS;
    AtrVertexFunc *texCoordSrcFunc[2];
    AtrVertexFunc *irgbSrcFunc;
    AtrVertexFunc *iaSrcFunc;
    AtrDstVertex destVertex[62]; /* This is the maximum chunk size */
} _AtrRenderCache;

#define ATR_CHUNK_SIZE 62

/* Light Callback */
struct _atrln;
typedef void (*_AtrLightFunc)( AtrDstVertex[],
                               AtrVertex[],
                               AtrVertex*,
                               struct _atrln* );


/*-------------------------------------------------------------------
  Structure: _AtrLightNode
  Date: 3/21/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Linked structure wrapper around AtrLights
  Members:
    light - an AtrLight
    tag - integer identifier for light
    vf - cached vector forward value
    lp - local position in O-space
    ld - cached light color * current diffuse material value
    ls - cached light color * current specular material value
    lf - pointer to light function
    next - the next _AtrLightNode
  -------------------------------------------------------------------*/
typedef struct _atrln {
    AtrLight      light;
    FxU32         tag;
    AtmVector3    vf;
    AtmVector3    lp;
    AtrColor      ld;
    AtrColor      ls;
    AtrXform      modelToProjector;
    float         fovXScale;
    float         fovYScale;
    _AtrLightFunc *lf;
    struct _atrln *next;
} _AtrLightNode;


extern _AtrLightFunc _atrLightTable[ATR_NUM_MAT_MODES]
                                   [ATR_NUM_LIGHT_TYPES]
                                   [ATR_NUM_ACCUM_MODES]
                                   [ATR_NUM_CLAMP_MODES]
                                   [ATR_NUM_MAT_TYPES];

extern _AtrRenderCache *_atrRenderCache;
extern AtrMaterial     *_atrRenderMaterial;
extern AtrCamera       *_atrCurrentCamera;
extern AtrXform        *_atrCurrentXform;
extern AtrCanvas       *_atrCurrentCanvas;
extern _AtrLightNode   *_atrLightHead;
extern AtrEnv          *_atrCurrentEnv;
extern AtrMaterial     *_atrCurrentMaterial;

extern AtrBuffer _atrRenderBuffer;

extern float _atrTwoFiftyFive;

typedef struct _AtrDriverTag AtrDriver;

/* TBD: need to implement clone image, some of these functions could be
        collapsed into 1 and the specific function set on a state change
 */

typedef struct _AtrDriverTag {
    AtrDriverCaps caps;
    size_t        texEntrySize;
    FxBool        (* RealizeImg)( AtrImg *i );
    void          (* UnrealizeImg)( AtrImg *i );
    FxBool        (* CloneImg)( AtrImg *dst, const AtrImg *src );
    void          (* RenderImg)( AtrImg *i, FxU32 winX, FxU32 winY);
    void          (* RenderImgBuffer)( AtrImg *i, FxU32 winX, FxU32 winY, FxU32 buffer);
    void          (* GrabImg)( AtrImg  *dst, FxU32  winX, FxU32 winY,
                           AtrBuffer buf);
    void          (* ClearCanvas)( float r, float g, float b, FxU16 za );
    void          (* SwapBuffer)( FxU32 sync ) ;
    FxBool        (* BeginScene)(FxU32 *viewWidth, FxU32 *viewHeight);
    FxBool        (* EndScene)(void);
    void          (* TexEntryInit)( _AtrTexEntry *e );
    void          (* TexDeleteHandle)( _AtrTexEntry *e);
    void          (* TexPunt)( _AtrTexEntry *pigskin );
    FxBool        (* TexAssociate)( AtrTexHandle handle, AtrImg *img );
    void          (* TexSource)( _AtrTexEntry *entry, FxU32 mask );
    void          (* TramAllocate)( _AtrTexEntry *entry );
    void          (* UpdateEnv)( AtrEnv *e );
    void          (* UpdateMaterial)( AtrMaterial *m );
    void          (* Shutdown)( void );
    void          (* Idle)( void );
    FxBool        (* IsDrawable)( AtrContext ctx );
    FxBool        (* Pause)(FxBool flag);
    FxBool        (* Resize)(AtrContext ctx, int w, int h) ;
    void          (* RenderBuffer)( AtrBuffer buf );
    void          (* DrawLine)( const GrVertex *a, const GrVertex *b );
    void          (* DrawPoint)( const GrVertex *p );
    void          (* DrawTriangle)(const GrVertex *a,const GrVertex *b,
                                 const GrVertex *c);
    void          (* Splash)( void );
    void          (* GetPerfStats)(GrSstPerfStats_t *pStats);
    void          (* ResetPerfStats)(void);
    void          (* ClipWindow)( int minx, int miny, int maxx, int maxy );
    FxU32         (* TransformVertices)( AtrDstVertex *dest, AtrVertex *src, 
                                         AtrVertex *end );
    void          (* SpecialTransformVertices)( AtrDstVertex *dest, AtrVertex *src, 
                                                                   AtrVertex *end );
    void          (* RenderTri)( AtrVertex *a, AtrVertex *b, AtrVertex *c );
    void          (* RenderTriSet)( AtrTriSet *t );
    void          (* SpecialRenderTriSet)( AtrTriSet *t );
    void          (* RenderTriWF)( AtrVertex *a, AtrVertex *b, AtrVertex *c );
    void          (* RenderTriSetWF)( AtrTriSet *t );
    void          (* TransformOpenTriSet)( AtrOpenTriSet *t );
    void          (* SpecialTransformOpenTriSet)( AtrOpenTriSet *t );
    void          (* RenderOpenTriSet)( AtrOpenTriSet *t );
    void          (* RenderOpenTriSetWF)( AtrOpenTriSet *t );
    void          (* RenderSegment)( AtrVertex *a, AtrVertex *b );
    void          (* TransformVertices2D)( AtrDstVertex *dest, AtrVertex *src,
                                           AtrVertex *end );
    void          (* Render2DTri)( AtrVertex *a, AtrVertex *b, AtrVertex *c );
    void          (* ClipAndRenderTri)( AtrDstVertex *a, AtrDstVertex *b,
                                        AtrDstVertex *c, FxU32 clipCode );
    void          (* ClipAndRenderSegment)(AtrDstVertex *a, AtrDstVertex *b,
                                          FxU32 clipCode );
    void          (* DitherMode)(AtrDitherMode mode);
    FxBool        (* Move)(AtrContext ctx, int x, int y) ;
} AtrDriver;

extern AtrDriver *_atrDriver, *_atrSavedDriver;

#define DRV_FUNC(_func) (_atrDriver->_func)

void _atrUpdateLights( AtrMaterial *m );
void _atrUpdateMaterial( AtrMaterial *m );
void _atrTexSetScale( FxU32 tmu, float s, float t );
void _atrTexSource( _AtrTexEntry *entry, FxU32 mask );
AtrTexelFx  _atrTexTmuFromHandle( AtrTexHandle t );
void _atrTexGetScale( FxU32 tmu, float *s, float *t );

void _atrTexPunt( _AtrTexEntry *pigskin );

extern _AtrTexEntry *(_inCache[2]);
extern _AtrTexEntry *(_outOfCache[2]);
extern _AtrTexEntry *(_unused[2]);

/*
**--------------------------------------------------------------
** Clipping
**--------------------------------------------------------------
*/

#define ATR_CC_BEHIND      0x01
#define ATR_CC_BELOW       0x02
#define ATR_CC_ABOVE       0x04
#define ATR_CC_LEFT        0x08
#define ATR_CC_RIGHT       0x10
#define ATR_CC_BEFORE      0x20
#define ATR_CC_MASK        0x3F
#define ATR_CC_CLIPPED     0x8000

void _atrClipAndRenderTri( AtrDstVertex *a,
                           AtrDstVertex *b,
                           AtrDstVertex *c,
                           FxU32 clipCode );
void _atrClipAndRenderTriWF( AtrDstVertex *a,
                             AtrDstVertex *b,
                             AtrDstVertex *c,
                             FxU32 clipCode );
void _atrClipAndRenderSegment(AtrDstVertex *a,
                              AtrDstVertex *b,
                              FxU32 clipCode );


/*
**--------------------------------------------------------------
** Statistics
**--------------------------------------------------------------
*/
extern AtrStats _atrCurrentStats;
void _atrStatsIncFrame(void);
void _atrStatsIncTris(FxU32 num);
void _atrStatsIncVerts(FxU32 num);
void _atrStatsIncClippedTris(FxU32 num);
void _atrStatsIncFFPixels( FxU32 pixels );
void _atrStatsIncTexDownloads( AtrTexelFx tmu );
void _atrStatsTexMemoryDownload( AtrTexelFx tmu, FxU32 size );
void _atrStatsTexMemoryCacheMiss( AtrTexelFx tmu );
void _atrStatsIncTexMem( AtrTexelFx tmu, FxU32 numBytes );
void _atrStatsDecTexMem( AtrTexelFx tmu, FxU32 numBytes );

#ifdef AT_PENTIUM_ASSEMBLY
extern FxU32 _cdecl _asm_TransformVertices( AtrDstVertex *dest, 
                                            AtrVertex *src,
                                            AtrVertex *end );
typedef FxU32 AtrXfFunc( AtrDstVertex *dest, 
                         AtrVertex *src,
                         AtrVertex *end );
#endif

AtrDriver *_atrXXXInit( AtrDriverInfo *driverInfo );
AtrDriver *_atrGlideInit( AtrDriverInfo *driverInfo );

ENDFDECL()

#endif



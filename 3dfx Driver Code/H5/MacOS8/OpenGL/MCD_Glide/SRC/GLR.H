/*________________________________________________________________________________________
** 
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
**________________________________________________________________________________________
**
**  Description: 
**
** 
**
*/


#ifndef _GLR_CONTEXT_H
#define _GLR_CONTEXT_H

#include <glide.h>
#include <g3ext.h>
#include <gsfc.h>

#include "gliMacSystem.h"
#include "gli.h"
#include "gld.h"
#include "glm.h"

#include "glr_debug.h"
#include "glr_debug_structs.h"
#include "glr_glide.h"

#define GLR_DRIVER_ID     0x1200

#define GLR_BUFFER_MAX 2048

#define GLR_VERTEX_MAX_COUNT 2048
#define GLR_NUM_VA_BUFFERS 2

/* Basic primitive types */
#define GLR_POINT           1
#define GLR_LINE            2
#define GLR_POLYGON         3

/* Hard-coded renderer capabilites */
#define GLR_MIN_LEVEL             0
#define GLR_MAX_LEVEL             0

#define GLR_MAX_ATTRIB_ARRAY_SIZE 36

#define GLR_TEXTURE_HASH_SIZE     512 /* must be (2^n) */
#define GLR_TEXTURE_HASH_MASK     GLR_TEXTURE_HASH_SIZE - 1

#define GLR_MAX_TEXTURE_LEVEL     12
#define GLR_NUM_TEXTURE_TARGETS   2
#define GLR_NUM_TEXTURE_UNITS     2

#define GLR_TMU0_ACTIVE (1L << GR_TMU0)
#define GLR_TMU1_ACTIVE (1L << GR_TMU1)

typedef struct glrTexture_s {
  GLenum
    target;    // either 1d or 2d
    
  GLuint
    texture_unit;  // for multi texturing extension
    
  GLsizei
    width;     // width of base level
    
  GLsizei
    height;    // height of base level
    
  GLsizei
    internal_width;     // width of base level
    
  GLsizei
    internal_height;    // height of base level
    
  GLint
    max_level;

  const GLDPerTextureState
    *state;

  GLint 
    largeLodLog2;
    
  GLint 
    smallLodLog2;

  GrTexInfo
    sys_texture;
    
  void
    *sys_data[GLR_MAX_TEXTURE_LEVEL];
      
  FxFloat
  	s_scale;		// opengl s * s_scale = glide s
	
  FxFloat
  	t_scale;		// opengl t * t_scale = glide t
	
  GLsizei
    size;
  
  struct glrTexture_s *
    next;

  GLenum 
    baseInternalFormat;
    
  GLboolean
    resident[GLR_NUM_TEXTURE_UNITS];

  GLint
    offset[GLR_NUM_TEXTURE_UNITS];
    
  struct glrTexture_s * 
    next_resident[GLR_NUM_TEXTURE_UNITS];
    
  struct glrTexture_s * 
    prev_resident[GLR_NUM_TEXTURE_UNITS];

} glrTexture_t;


struct cmdTransportInfo {
    FxU32  triPacketHdr; /* Pre-computed packet header for
                          * independent triangles. 
                          */
    
    FxU32  cullStripHdr; /* Pre-computed packet header for generic
                          * case of packet 3 triangles. This needs
                          * command type and # of vertices to be complete.
                          */
    
    FxU32  paramMask;    /* Mask for specifying parameters of
                          * non-triangle packets.  The parameter
                          * bits[21:10] mimic the packet3 header
                          * controlling which fields are sent, and
                          * pc[28] controls whether any color
                          * information is sent as packed.
                          */
    
    FxU32* fifoPtr;      /* Current write pointer into fifo */
    FxU32  fifoRead;     /* Last known hw read ptr. 
                          * If on an sli enabled system this will be
                          * the 'closest' hw read ptr of the sli
                          * master and slave.
                          */
    
    /* Fifo checking information. In units of usuable bytes until
     * the appropriate condition.
     */
    FxI32  fifoRoom;     /* Space until next fifo check */

    FxBool autoBump;     /* Are we auto bumping (aka hole counting?) */
    FxU32
      *lastBump,         /* Last ptr where we bumped. */
      *bumpPos;          /* Next place to bump */
    FxU32
      bumpSize;          /* # of DWORDS per bump */

    FxU32 **ptrLostContext;

    /* Internal bookkeeping information - Everything after this point
     * is not exposed via the COMMAND_TRANSPORT extension 
     */

    /* Basic command fifo characteristics. These should be
     * considered logically const after their initialization.
     */
    FxU32* fifoStart;    /* Virtual address of start of fifo */
    FxU32* fifoEnd;      /* Virtual address of fba fifo */
    FxU32  fifoOffset;   /* Offset from hw base to fifo start */
    FxU32  fifoSize;     /* Size in bytes of the fifo */
    FxU32  fifoJmpHdr[2];/* Type0 packet for jmp to fifo start
                          * only first DWORD is used for memory
                          * fifo--both are used for AGP FIFO
                          */

};

#define GLR_VA_TYPE_LAST 7

typedef void (*glrVertexArrayRenderingFuncPtr)(GLDContext ctx, void* data, GLint first, GLsizei count, const GLuint *indices);

typedef struct glrVertexArray_s {

  GLenum
    dataSize[GLR_NUM_VA_BUFFERS];

  glrVertexArrayRenderingFuncPtr *
    renderFuncTbl;
    
  void *
    data[GLR_NUM_VA_BUFFERS];
  
  GLuint
    current_va_index;
      
} glrVertexArray_t;



// entire context rec is initialized to 0 in gldCreateContext
struct GLDContextRec {

  /* Drawable info and position */
  GLIDrawable
    drawable;
  GLenum
    drawableType;
  GLIDevice  
	device;
  	
  GLDViewport
    viewPort;
  GLboolean
  	viewPortDirty;
  CGrafPtr
    targetPort,
	renderPort;
  GrSurface_t
    renderSurface;
  GrSurface_t
    auxSurface;
  GrSurface_t
    texture_surface;
  GrSurfaceDesc_t
    renderSurfaceDesc;
  GrSurfaceDesc_t
    auxSurfaceDesc;
  GrSurfaceDesc_t
    textureSurfaceDesc;
  GrContext_t
    glideContext;
  GLboolean
	surfaceLost;
  FxU32
    glideBoardSelectID,
    wRange[2];

  GLTRectangle
    buffer_rect,
    window_rect,
    swap_rect;
  GLboolean
    buffer_rect_enabled,
	swap_rect_enabled;
	
  GLint
    swap_interval;
    	
  /* OpenGL rendering pipeline state data */
  GLboolean
    context_reinit;
  const GLDState *
    state;
  GLfloat
    fill_z_offset;
  GLenum
    primitive;	
  GrBuffer_t
    readBuffer;
    
  /* Context user selection */
  GLboolean
    has_depth;

  /* textures */
  FxU32
    max_texture_lod;  // Biggest texture size allowed by this context.
    
  glrTexture_t *
    textures[GLR_TEXTURE_HASH_SIZE];    // hashed list of named textures. 

  GLuint
    cur_texture_name[GLR_NUM_TEXTURE_UNITS][GLR_NUM_TEXTURE_TARGETS];

  /* Note: cur_texture array refers to hardware texture units instead of logical texture units */
  glrTexture_t *
    hw_texture[GLR_NUM_TEXTURE_UNITS];
		
  /* This maps from the two ARB texture units to the two hardware texture units */
  GLuint
    arb_unit_map[GLR_NUM_TEXTURE_UNITS];

  struct cmdTransportInfo *
    cmdTransportInfo;
    
  // to do: move these fields to a per-board record
  glrTexture_t * 
    first_resident_texture[GLR_NUM_TEXTURE_UNITS];     // first texture in vram
  glrTexture_t * 
    latest_resident_texture[GLR_NUM_TEXTURE_UNITS];    // most recently allocated texture
  FxI32
    surface_size[GLR_NUM_TEXTURE_UNITS];
  GLboolean
    isUMA;
    
  /* set error */
  GLenum
    error;

  /* GLI */
  GLIPixelFormat
    gliPixelFormat;
    
  FxU32
    grPixelFormat;
    
  GLDRenderDispatch
    *dispatch_table;

  /* vertex arrays */
  GLenum
    currentVertexDataType;

  glrVertexArrayRenderingFuncPtr *
    currentVertexDataTypeRenderFuncTbl;

  glrVertexArray_t
    va[ GLR_VA_TYPE_LAST ];
  
  GLuint
    *indexBuffer;
  
  GLuint
    indexBufferSize;
  
  GLuint
    intraFrameCounter;
    
  GLuint
    cpuType;	// 1 = has altivec, 0 = does not have altivec
    
  void
    (*configureCombineUnits)(GLDContext ctx, GLuint activeTextureUnits);

  glrGlideExtensions_t
    glideExtensions;

  // state optimization flags
  GLuint combineUnitState;
};





/*
____________________________________________________________________________ GLD API ___
*/




/*_____ Exported GLD Functions Definitions _____*/

extern void glrSetError(GLDContext ctx, GLenum err);

extern GLboolean glrValidatePixelFormat(const GLIPixelFormat *format);

/*_____ from glr_texture.c _____*/
extern void glrFreeAllTextures(GLDContext ctx);
extern void glrFreeAllTexturesVRAM(GLDContext ctx);
extern GLuint glrLoadCurrentTexture(GLDContext ctx);


/*_____*/

extern void glrNopDispatchTable(GLDContext inContext);
extern void glrNotImplemented(void);

extern void glrUpdateHardwareState(GLDContext ctx, GLbitfield change_flags);

extern void gldFinish(GLDContext ctx);
extern void gldFlush(GLDContext ctx);

extern void gldSwapBuffers(GLDContext ctx);


/* Internal dispatched functions */
extern void glrConfigureCombineUnitsSST1(GLDContext ctx, GLuint activeTextureUnits);
extern void glrConfigureCombineUnitsNapalm(GLDContext ctx, GLuint activeTextureUnits);


/*_____ Dispatched GLD Rendering Functions Definitions _____*/

extern void gldAccum(GLDContext inContext, GLenum inOp, GLfloat inValue);
extern void gldClear(GLDContext ctx, GLbitfield mask);

extern void glrReadPixels(GLDContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels, GLboolean packed);
extern void glrDrawPixels(GLDContext ctx, const GLDVertex *rpos, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels, GLboolean packed);
extern void glrCopyPixels(GLDContext ctx, const GLDVertex *rpos, GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);

extern void glrRenderBitmap(GLDContext ctx, const GLDVertex *rpos, GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLuint name, const GLubyte *bitmap, GLboolean packed);

extern void glrRenderPoints(GLDContext ctx, const GLDVertex *v_stack, GLint num_v);
extern void glrRenderPointsPtr(GLDContext ctx, const GLDVertex **v_stack, GLint num_v);

extern void glrRenderFlatLines(GLDContext ctx, const GLDVertex *v_stack, GLint num_v);
extern void glrRenderSmoothLines(GLDContext ctx, const GLDVertex *v_stack, GLint num_v);
extern void glrRenderFlatLinesPtr(GLDContext ctx, const GLDVertex **v_stack, GLint num_v, GLenum reset);
extern void glrRenderSmoothLinesPtr(GLDContext ctx, const GLDVertex **v_stack, GLint num_v, GLenum reset);
extern void glrRenderFlatLineStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLenum reset);
extern void glrRenderSmoothLineStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLenum reset);
extern void glrRenderFlatLineLoop(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLenum reset);
extern void glrRenderSmoothLineLoop(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLenum reset);

extern void glrRenderFlatTriangles(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothTriangles(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatTriangleFan(GLDContext ctx, const GLDVertex *v_pivot, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothTriangleFan(GLDContext ctx, const GLDVertex *v_pivot, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatTriangleStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothTriangleStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatQuads(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothQuads(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatQuadStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothQuadStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatPolygon(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothPolygon(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatPolygonPtr(GLDContext ctx, const GLDVertex **v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothPolygonPtr(GLDContext ctx, const GLDVertex **v_stack, GLint num_v, GLbitfield options);

extern void glrRenderFlatTextureTriangles(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatMultiTextureTriangles(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothTextureTriangles(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothMultiTextureTriangles(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatTextureTriangleFan(GLDContext ctx, const GLDVertex *v_pivot, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatMultiTextureTriangleFan(GLDContext ctx, const GLDVertex *v_pivot, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothTextureTriangleFan(GLDContext ctx, const GLDVertex *v_pivot, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothMultiTextureTriangleFan(GLDContext ctx, const GLDVertex *v_pivot, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatTextureTriangleStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatMultiTextureTriangleStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothTextureTriangleStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothMultiTextureTriangleStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);

extern void glrRenderFlatTextureQuads(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatMultiTextureQuads(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothTextureQuads(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothMultiTextureQuads(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatTextureQuadStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatMultiTextureQuadStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothTextureQuadStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothMultiTextureQuadStrip(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);

extern void glrRenderFlatTexturePolygon(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatMultiTexturePolygon(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothTexturePolygon(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothMultiTexturePolygon(GLDContext ctx, const GLDVertex *v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatTexturePolygonPtr(GLDContext ctx, const GLDVertex **v_stack, GLint num_v, GLbitfield options);
extern void glrRenderFlatMultiTexturePolygonPtr(GLDContext ctx, const GLDVertex **v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothTexturePolygonPtr(GLDContext ctx, const GLDVertex **v_stack, GLint num_v, GLbitfield options);
extern void glrRenderSmoothMultiTexturePolygonPtr(GLDContext ctx, const GLDVertex **v_stack, GLint num_v, GLbitfield options);

extern void glrRenderVertexBuffer(GLDContext ctx, void *data, GLbitfield options, GLenum mode, GLint first, GLsizei count, GLenum type, const void *indices);
extern void glrEndPrimitiveBuffer(GLDContext ctx, void *data, GLbitfield options, GLenum mode, GLsizei count);

extern void glrVARenderTextureTrianglesG3(GLDContext ctx, void *data, GLint first, GLsizei count, const GLuint *indices);
extern void glrVARenderSmoothPolygonsG3(GLDContext ctx, void *data, GLint first, GLsizei count, const GLuint *indices);
extern void glrVARenderTexturePolygonsG3(GLDContext ctx, void *data, GLint first, GLsizei count, const GLuint *indices);
extern void glrVARenderMultiTextureTrianglesG3(GLDContext ctx, void *data, GLint first, GLsizei count, const GLuint *indices);
extern void glrVARenderMultiTexturePolygonsG3(GLDContext ctx, void *data, GLint first, GLsizei count, const GLuint *indices);

extern void glrVARenderTextureTrianglesG4(GLDContext ctx, void *data, GLint first, GLsizei count, const GLuint *indices);
extern void glrVARenderSmoothPolygonsG4(GLDContext ctx, void *data, GLint first, GLsizei count, const GLuint *indices);
extern void glrVARenderTexturePolygonsG4(GLDContext ctx, void *data, GLint first, GLsizei count, const GLuint *indices);
extern void glrVARenderMultiTextureTrianglesG4(GLDContext ctx, void *data, GLint first, GLsizei count, const GLuint *indices);
extern void glrVARenderMultiTexturePolygonsG4(GLDContext ctx, void *data, GLint first, GLsizei count, const GLuint *indices);

// mac utility functions
void glrGetWindowGlobalRect(CWindowPtr window, Rect * globalRect);
extern GDHandle glrGetWindowDevice(CWindowPtr window);
GLint glrDeviceToBoardID(GDHandle gd);


#endif /* _GLR_CONTEXT_H */
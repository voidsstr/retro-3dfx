#ifndef ATD3D_H
#define ATD3D_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ddraw.h>
#include <d3d.h>
#include "atrender.h"
#include "fxatr.h"

extern const float _atrD3D_ZSCALE;

#ifdef DPRIMAPI
#define ATD3D_MAX_VERTS 100
#define ATD3D_MAX_IDX  1000
extern D3DTLVERTEX _atD3DVerts[ATD3D_MAX_VERTS];
extern WORD _atD3DIdx[ATD3D_MAX_IDX];
extern int _atd3dNumVerts, _atd3dCurVert;
extern int _atd3dNumIdx, _atd3dCurIdx;
extern int _atd3dGeomType;
void _atrD3DFlushGeom(void);
void _atrD3DCheckGeom(D3DPRIMITIVETYPE type, int numVerts, int numIdx);
#endif

#define WINDOW_WIDTH  320
#define WINDOW_HEIGHT 200
#define CHECK(c,s) {if((c))\
{dprintf("%s\n%s: line%d",s,__FILE__,__LINE__); return(FALSE);}}
#define CHECK_NO_RETURN(c,s) {if((c))\
{dprintf("%s\n%s: line%d",s,__FILE__,__LINE__);}}
#define LINE_INFO dprintf("Line %d in %s\n", __LINE__, __FILE__);
#define DEBUG_1 0
#define MAX_TEXTURES 100
#define VIDEO_MEMORY_MISS   -1
#define HANDLE_MISS -1

/*********************************Miscellaneous****************************/
void __cdecl dprintf(char *format, ...);

#define MAX_DDDRIVERS   8
#define MAX_D3DDRIVERS  8
#define MAX_NAME_LENGTH 64
#define MAX_CONTEXTS 1

typedef struct
{
  GUID Guid;
  char DriverDesc[MAX_NAME_LENGTH];
  char DriverName[MAX_NAME_LENGTH];
  int attachment;     /*place to store index to DD object for D3D object*/
} DriverInfo;

#define NO_OPCODE 255

/*************************************************************************/
#define D3DExit(ptr) D3DInstruction((ptr), D3DOP_EXIT, 0, 0)
/*************************************************************************/

typedef struct
{
  /*pointers into execution buffer data*/
  char*  lpBufStart;
  char*  lpVertexPointer;
  char*  lpInstStart;
  char*  lpInstPointer;
  char*  lpInstEnd ;
  BYTE   lastOpcode; /*bOpcode in D3DINSTRUCTION struct*/
  WORD  *lastCount;  /*wCount in D3DINSTRUCTION struct*/
  size_t size;
  int    maxVertices;
  int    numVertices;
} EBufferData, *EBufferDataPtr;

/*-------------------------------------------------------------------
  Structure: AtrD3DImg
  Date: 10/14
  Implementor(s): mlwp
  Library: AT Render
  Description: 
  Structure for storing D3D specific image data.
  data    - raw image data, upper left pixel to bottom right in 
            row order
  table   - pointer to AtrImgTable union for paletized/compressed 
            images
  -------------------------------------------------------------------*/
typedef struct {
    LPDIRECTDRAWSURFACE lpSurface; /* system memory image of texture */
    LPDIRECT3DTEXTURE2  lpTexture; /* texture in system memory */
    LPDIRECTDRAWPALETTE lpPalette ;
} AtrD3DImg;

typedef struct _ATRD3DTE {
    _AtrTexEntry        any; /* must be first entry */
    LPDIRECTDRAWPALETTE lpPalette ;
    D3DTEXTUREHANDLE    texHandle;    /* handle to video memory texture */
    LPDIRECTDRAWSURFACE smSurf;  /* surface in video memory */
    LPDIRECT3DTEXTURE2  smTexture; /* texture in system memory */
    LPDIRECTDRAWSURFACE vmSurf;  /* surface in video memory */
    LPDIRECT3DTEXTURE2  vmTexture; /* video memory texture */
} _AtrD3DTexEntry;

/*graphics context structure */

typedef struct {
    AtrDriver                 any; /* this MUST be first element in structure */
    AtrBuffer                 renderBuffer;
    int                       texFormatI8 ;
    int                       texFormat565 ;
    int                       texFormat4444 ;
    LPDIRECT3DDEVICE          lpD3DDevice;
    LPDIRECT3DVIEWPORT        lpD3DViewport;
    LPDIRECT3DDEVICE2         lpD3DDevice2;
    LPDIRECT3DVIEWPORT2       lpD3DViewport2;
  
    LPDIRECTDRAWSURFACE       FrontBuffer;
    LPDIRECTDRAWSURFACE       BackBuffer;
    LPDIRECTDRAWSURFACE       ZBuffer;

    /* background material */

    LPDIRECT3DMATERIAL2 lpBmat;
    D3DMATERIAL bmat;
    D3DMATERIALHANDLE hBmat;
  
    DWORD TotalVideoMemory, FreeVideoMemory;
    DWORD TotalTextureMemory, FreeTextureMemory;
  
    LPDIRECT3DEXECUTEBUFFER lpD3DExBuf;
    D3DEXECUTEBUFFERDESC debDesc;
    D3DEXECUTEDATA d3dExData;
    int MaxBufferSize;
    int MaxVertexCount;
  
    int TexturesOn;     /*Are we texture mapping now*/
  
    DDSURFACEDESC ddsdTextureFormat;
  
    DriverInfo DDDriver[MAX_DDDRIVERS], D3DDriver[MAX_D3DDRIVERS];
    int NumDrivers[2], Selection[2];
  
    int ConstantColorValue;
    int FlipY;  /*Should we flip the screen to be consistent with Glide*/
  
    int clip_minx;
    int clip_miny;
    int clip_maxx;
    int clip_maxy;  
  
    EBufferData ebd;
} _AtrD3DDriver;

/* graphics contexts for d3d driver */

extern _AtrD3DDriver d3dContexts[MAX_CONTEXTS];
extern _AtrD3DDriver *d3dContext;

/**********Functions to build the execution buffer*****************/
void D3DInstruction(_AtrD3DDriver *ctx, BYTE opcode, BYTE size, WORD count);
void D3DRenderState(_AtrD3DDriver *ctx, D3DRENDERSTATETYPE type, DWORD arg);
void D3DRenderStatef(_AtrD3DDriver *ctx, D3DRENDERSTATETYPE type, D3DVALUE v);
void D3DLightState(_AtrD3DDriver *ctx, D3DLIGHTSTATETYPE type, DWORD arg);
void D3DLightStatef(_AtrD3DDriver *ctx, D3DLIGHTSTATETYPE type, D3DVALUE v);
void D3DProcessVertices(_AtrD3DDriver *ctx, DWORD flags, DWORD count);
void D3DTriangle(_AtrD3DDriver *ctx, int v1, int v2, int v3, FxU16 flags);
FxU32 _atrD3DTransformVertices( AtrDstVertex *dest, AtrVertex *src, 
                                 AtrVertex *end );
FxU32 atr_d3d_transform_verts_asm( AtrDstVertex *dest, AtrVertex *src, 
                                 AtrVertex *end );
void _atrD3DSpecialTransformVertices( AtrDstVertex *dest, AtrVertex *src, 
                                        AtrVertex *end );
void _atrD3DTransformVertices2D( AtrDstVertex *dest, AtrVertex *src, 
                                   AtrVertex *end );
void atrD3DRenderTri( AtrVertex *a, AtrVertex *b, AtrVertex *c );
void atrD3DRenderTriSet( AtrTriSet *t );
void atrD3DSpecialRenderTriSet( AtrTriSet *t );
void atrD3DRenderTriWF( AtrVertex *a, AtrVertex *b, AtrVertex *c );
void atrD3DRenderTriSetWF( AtrTriSet *t );
void atrD3DTransformOpenTriSet( AtrOpenTriSet *t );
void atrD3DSpecialTransformOpenTriSet( AtrOpenTriSet *t );
void atrD3DRenderOpenTriSet( AtrOpenTriSet *t );
void atrD3DRenderOpenTriSetWF( AtrOpenTriSet *t );
void atrD3DRenderSegment( AtrVertex *a, AtrVertex *b );
void atrD3DRender2DTri( AtrVertex *a, AtrVertex *b, AtrVertex *c );

void _atrD3DClipAndRenderTri( AtrDstVertex *a, AtrDstVertex *b,
                              AtrDstVertex *c, FxU32 clipCode );
void _atrD3DClipAndRenderSegment(AtrDstVertex *a, AtrDstVertex *b,
                                 FxU32 clipCode );
void _atrD3DCheckBufferFull(_AtrD3DDriver *ctx, int numVerts, 
                            int numInstr, size_t misc);
void _atrD3DCheckAlignment(_AtrD3DDriver *ctx);
void _atrD3DProcessVertices(_AtrD3DDriver *ctx, DWORD flags, DWORD count);
void _atrD3DInstruction(_AtrD3DDriver *ctx, BYTE opcode, BYTE size, WORD count);

/* Rendering Callbacks  */
AtrVertexFunc _atrD3D_TCSRC0_TC0;
AtrVertexFunc _atrD3D_TCSRC0_TC1;
AtrVertexFunc _atrD3D_TCSRC0_TC2;
AtrVertexFunc _atrD3D_TCSRC0_PROJECTED;
AtrVertexFunc _atrD3D_TCSRC0_PLANAR;
AtrVertexFunc _atrD3D_TCSRC0_EMAP;
AtrVertexFunc _atrD3D_TCSRC1_TC0;
AtrVertexFunc _atrD3D_TCSRC1_TC1;
AtrVertexFunc _atrD3D_TCSRC1_TC2;
AtrVertexFunc _atrD3D_TCSRC1_EMAP;
AtrVertexFunc _atrD3D_TCSRC1_PROJECTED;
AtrVertexFunc _atrD3D_TCSRC1_PLANAR;

AtrVertexFunc _atrTCSRC0_2DTC0;

#ifdef __cplusplus
};
#endif

#endif

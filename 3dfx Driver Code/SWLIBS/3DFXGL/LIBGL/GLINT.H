/* 
** Copyright (c) 1997, 3Dfx Interactive, Inc. 
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
** 
** 
*/ 
#include <glide.h>
#include <GL/glaux.h>

#define glDegtoRad (3.14159265358979323846/180.0)

#define ColorubtoFloat(val) ((float)(val))/255.0f
#define ColorbtoFloat(val)  ((((float)(val))*2.0f)+1.0f)/255.0f
#define ColorustoFloat(val) ((float)(val))/65535.0f
#define ColorstoFloat(val) ((((float)(val))*2.0f)+1.0f)/65535.0f
#define ColoruitoFloat(val) ((float)(val))/((65536.0f*65536.0f)-1.0f)
#define ColoritoFloat(val) ((((float)(val))*2.0f)+1.0f)/((65536.0f*65536.0f)-1.0f)
#define ColorftoFloat(val) (val)
#define ColordtoFloat(val) ((float)(val))

#define LighttoFloat(val) ((float)val)/((65536.0f*65536.0f)/2.0f)

//#ifdef RMG
#define GLINT_OUTSIDE_BEGIN() if((pglCurContext->BeginEnd == TRUE) && \
                                   (pglCurContext->GLerr == GL_NO_ERROR)) \
                                     pglCurContext->GLerr = \
                                       GL_INVALID_OPERATION

#define GLINT_INSIDE_BEGIN() if((pglCurContext->BeginEnd == FALSE) && \
                                  (pglCurContext->GLerr == GL_NO_ERROR)) \
                                     pglCurContext->GLerr = \
                                       GL_INVALID_OPERATION

#define GLINT_ERROR(val) if(pglCurContext->GLerr == GL_NO_ERROR) \
                            pglCurContext->GLerr = val
//#endif

#ifdef RMG
void GLINT_ERROR(int);
void GLINT_OUTSIDE_BEGIN(void);
void GLINT_INSIDE_BEGIN(void);
#endif

#define GLINT_PARAM(param,val,format) \
    glIntRead((void *)(param), (void *)&(val), type, (format))

#define GLINT_PARAMC(param,val) \
    glIntReadC((void *)(param), (val), type)

#define GLINT_LIST_SIZE 8192

#define SNAP_BIAS ((float)(3<<18))
#define CLIP_MIN_Z 0.0f

#define GR_TEXFMT_DEPTH 0xff

#define OUT_MINUS_X 0x001
#define OUT_PLUS_X  0x002
#define OUT_MINUS_Y 0x004
#define OUT_PLUS_Y  0x008
#define OUT_MINUS_Z  0x010
#define OUT_PLUS_Z  0x020
#define OUT_CLIP0   0x040
#define OUT_CLIP1   0x080
#define OUT_CLIP2   0x100
#define OUT_CLIP3   0x200
#define OUT_CLIP4   0x400
#define OUT_CLIP5   0x800

#define READ_DOUBLE 0x0
#define READ_FLOAT 0x1
#define READ_FLOATC 0x2
#define READ_INT 0x3
#define READ_SHORT 0x4
#define READ_BYTE 0x5
#define READ_BOOL 0x6

#define NUM_FAST_TEX 512

#define LI_VERTEX_COLOR 0x0
#define LI_FRONT_COLOR 0x1
#define LI_BACK_COLOR 0x2

typedef struct {
  GLfloat Ambient[3];
  GLfloat Diffuse[4];
  GLfloat Specular[3];
  GLfloat Position[4];
  GLfloat SpotDirection[4];
  GLfloat SpotExponent;
  GLfloat SpotCutoff;
  GLfloat ConstantAtten;
  GLfloat LinearAtten;
  GLfloat QuadraticAtten;

  // Lighting Validation
  GLfloat FrontAmbient[3];
  GLfloat BackAmbient[3];
  GLfloat FrontDiffuse[3];
  GLfloat BackDiffuse[3];
  GLfloat FrontSpecular[3];
  GLfloat BackSpecular[3];

  GLboolean NoAtten;
  GLboolean NoSpot;
  GLfloat Cosine;
} glLight;

typedef struct {
  GLboolean Lighted;
  GLboolean Projected;
  GLfloat Color[4];
  GLfloat Clip[4];
  GLfloat EyePos[4];
  GLfloat TxNormal[3];
  GLfloat Texture[4];
  int OutCodes;
  GLboolean Edge;
  GrVertex Glide;
} glLocalVertex;

typedef struct sglTexture {
  GLenum Type;
  struct sglTexture *CacheFwd;
  struct sglTexture *CacheBack;
  GLboolean Cached;
  int CachedSize;
  unsigned int Address;
  GLboolean Invalid;
  int InternalFormat;
  int Width;
  int Height;
  int BorderWidth;
  GLfloat BorderColor[4];
  int SWrap;
  int TWrap;
  int Mag;
  int Min;
  GLfloat Priority;
  GLfloat SScale;
  GLfloat TScale;
  GLboolean TexFromList;
  unsigned char *Level[9];
  
  // Glide State
  int GlideSize;
  GrTexInfo GlideTex;
  GrTextureClampMode_t GlideSWrap;
  GrTextureClampMode_t GlideTWrap;
  GrTextureFilterMode_t GlideMin;
  GrTextureFilterMode_t GlideMag;
  GrMipMapMode_t GlideMIPMode;
  FxBool GlideLodBlend;

} glTexture, *pglTexture;

typedef struct sglTexBlock {
  struct sglTexBlock *Next; 
  GLuint TexName;
  pglTexture Texture;
} glTexBlock, *pglTexBlock;

typedef struct sglListBlock {
  struct sglListBlock *Next; 
  GLuint ListName;
  unsigned int *ListBlocks;
  unsigned int *DataBlocks;
} glListBlock, *pglListBlock;

typedef struct {
  GLbitfield mask;

  // Start Current
  GLfloat CurColor[4];
  GLfloat CurTex[4];
  GLfloat CurNormal[3];

  GLfloat CurIndex;

  GLfloat RasObj[4];
  // Raster Eye Distance
  GLfloat RasRed;
  GLfloat RasGreen;
  GLfloat RasBlue;
  GLfloat RasAlpha;
  GLfloat RasTex[4];
  GLboolean RasValid;
  // End Current
  
  // Start Viewport
  GLint ViewX;
  GLint ViewY;
  GLsizei ViewWidth;
  GLsizei ViewHeight;
  GLfloat xScale;
  GLfloat xOffset;
  GLfloat yScale;
  GLfloat yOffset;
  GLfloat zScale;
  GLfloat zOffset;

  GLfloat zNear;
  GLfloat zFar;
  // End Viewport

  // Start Transform
  GLenum MatMode;
  GLfloat ClipDef[6][4];
  GLfloat ClipXform[6][4];
  // End Transform

  // Start Transform/Enable
  GLboolean Normalize;
  GLboolean ClipEna[6];
  // End Transform/Enable

  // Start Fog
  GrFog_t FogTable[GR_FOG_TABLE_SIZE];
  GLenum FogMode;
  GLfloat FogDensity;
  GLfloat FogStart;
  GLfloat FogEnd;
  GrColor_t FogColor;
  // End Fog
  
  // Start Fog/Enable
  GLboolean Fog;
  // End Fog/Enable

  // Start Lighting
  GLenum ShadeModel;
  GLenum ColorMaterialMode;
  GLenum ColorMaterialFace;
  GLfloat FrontAmbient[4];
  GLfloat FrontDiffuse[4];
  GLfloat FrontSpecular[4];
  GLfloat FrontEmission[4];
  GLfloat FrontShininess;
  GLfloat BackAmbient[4];
  GLfloat BackDiffuse[4];
  GLfloat BackSpecular[4];
  GLfloat BackEmission[4];
  GLfloat BackShininess;
  GLfloat LightModelAmbient[4];
  GLboolean Local_Viewer;
  GLboolean Two_Sided;

  glLight Light[8];
  // End Lighting

  // Start Lighting/Enable
  GLboolean Lighting;
  GLboolean ColorMaterial;
  GLboolean LightEna[8];
  // End Lighting/Enable

  // Start Point
  GLfloat PointWidth;
  // End Point

  // Start Point/Enable
  GLboolean PointAA;
  // End Point/Enable

  // Start Line
  GLint LineStippleFactor;
  GLushort LinePattern;
  GLfloat LineWidth;
  // End Line

  // Start Line/Enable
  GLboolean LineAA;
  GLboolean LineStipple;
  // End Line Enable

  // Start Polygon
  GLenum CullMode;
  GLenum FrontFace;
  GLenum PolygonFrontMode;
  GLenum PolygonBackMode;
  // Polygon Offset
  // Polygon Bias
  // End Polygon

  // Start Polygon/Enable
  GLboolean CullEnable;
  GLboolean PolyAA;
  // Polygon offset point
  // Polygon offset line
  // Polygon offset fill
  GLboolean PolyStipple;
  // End Polygon/Enable

  // Start Polygon Stipple
  // Polygon stipple value
  // End Polygon Stipple

  // Start Texture
  GLuint Tex1DName;
  pglTexture Tex1DPtr;
  GLuint Tex2DName;
  pglTexture Tex2DPtr;
  GLfloat TexEnvColor[4];
  GLint TexMode;
  // Texture Priority
  // Texture Eye Linear
  // Texture Object Linear
  // Texture Gen Mode
  // End Texture

  // Start Texture/Enable
  GLboolean Tex1D;
  GLboolean Tex2D;
  // Texture Gen
  // End Texture/Enable

  // Start Scissor
  GLint minx;
  GLint miny;
  GLint maxx;
  GLint maxy;
  // End Scissor

  // Start Scissor/Enable
  GLboolean ScissorEnable;
  // End Scissor/Enable

  // Start Color-Buffer
  GLenum AlphaFunc;
  GrCmpFnc_t gAlphaFunc;
  GrAlpha_t AlphaRef;
  GLenum SrcBlend;
  GrAlphaBlendFnc_t gSrcBlend;
  GLenum DstBlend;
  GrAlphaBlendFnc_t gDstBlend;
  // Logic OP Mode
  GLenum DrawBuffer;
  GLboolean RedMask; 
  GLboolean GreenMask; 
  GLboolean BlueMask; 
  GLboolean AlphaMask; 
  // Index Mask
  GLfloat ColorClrRed;
  GLfloat ColorClrGreen;
  GLfloat ColorClrBlue;
  GrColor_t ClrColor;
  GLfloat ColorClrAlpha;
  GrAlpha_t ClrAlpha;
  GLfloat ClrIndex;
  // End Color-Buffer

  // Start Color-Buffer/Enable
  GLboolean AlphaTest;
  GLboolean Blend;
  // Dither
  // Index Logic OP
  // Color Logic OP
  // End Color-Buffer/Enable

  // Start Stencil-Buffer
  GLenum StencilZPass;
  GLenum StencilZFail;
  GLenum StencilFail;
  GLenum StencilFunc;
  GLuint StencilFuncMask;
  GLint StencilRef;
  GLuint StencilMask;
  GLint ClrStencil;
  // Stencil Mask
  // End Stencil-Buffer

  // Start Stencil-Buffer/Enable
  GLboolean StencilTest;
  // End Stencil-Buffer/Enable

  // Start Depth-Buffer
  GLenum DepthFunc;
  GrCmpFnc_t gDepthFunc;
  GLclampd GLClrDepth;
  FxU16 ClrDepth;
  GLboolean DepthMask; 
  // End Depth-Buffer

  // Start Depth-Buffer/Enable
  GLboolean DepthBuffer;
  // End Depth-Buffer/Enable

  // Start Accum-Buffer
  GLfloat AccumClrRed;
  GLfloat AccumClrGreen;
  GLfloat AccumClrBlue;
  GLfloat AccumClrAlpha;
  // End Accum-Buffer

  // Start List
  GLuint ListBase;
  // End List

  // Start Eval
  // End Eval

  // Start Eval/Enable
  // End Eval/Enable

  // Start Hint
  // End Hint

  // Start Pixel
  GLboolean PixelMapColor;
  GLboolean PixelMapStencil;
  GLint PixelIndexShift;
  GLint PixelIndexOffset;
  GLfloat PixelRedScale;
  GLfloat PixelRedBias;
  GLfloat PixelGreenScale;
  GLfloat PixelGreenBias;
  GLfloat PixelBlueScale;
  GLfloat PixelBlueBias;
  GLfloat PixelAlphaScale;
  GLfloat PixelAlphaBias;
  GLfloat PixelDepthScale;
  GLfloat PixelDepthBias;
  GLfloat PixelZoomX;
  GLfloat PixelZoomY;
  GLfloat PixelMaps[10][256];
  GLint PixelMapSize[10]; 
  GLenum ReadBuffer;
  // End Pixel
} glAttrib, *pglAttrib;

typedef struct {

  // Window state
  int gltype;
  HDC hdc;
  HWND hwnd;

  // Buffer Masks
  GLboolean RedMask; 
  GLboolean GreenMask; 
  GLboolean BlueMask; 
  GLboolean AlphaMask; 

  GLboolean DepthMask; 

  // Clear state
  GLfloat AccumClrRed;
  GLfloat AccumClrGreen;
  GLfloat AccumClrBlue;
  GLfloat AccumClrAlpha;

  GLfloat ColorClrRed;
  GLfloat ColorClrGreen;
  GLfloat ColorClrBlue;
  GrColor_t ClrColor;

  GLfloat ColorClrAlpha;
  GrAlpha_t ClrAlpha;

  GLclampd GLClrDepth;
  FxU16 ClrDepth;

  GLfloat ClrIndex;

  GLint ClrStencil;

  // Vertex state
  GLfloat *CurColor;
  GLfloat *CurTex;
  GLfloat *CurNormal;

  GLfloat LocCurColor[4];
  GLfloat LocCurTex[4];
  GLfloat LocCurNormal[4];

  GLfloat CurIndex;

  GLfloat *CurObj;
  GLfloat LocCurObj[4];

  GLfloat CurEye[4];
  GLfloat CurClip[4];

  GLboolean CurEdge;

  GLfloat CurTxNormal[4];

  GLfloat CurTxTex[4];

  glLocalVertex Vtx1, Vtx2, Vtx3, Vtx4;

  glLocalVertex *CurVtx;

  GLenum PrimAssemState;
  GLboolean CurLine;
  GLboolean CurPoly;

  GLboolean CurFrontFace;
  GLenum FrontFace;

  GLenum CullMode;

  GLboolean BeginEnd;
  GLenum GLerr;

  // Vertex Array
  GLboolean VertexEnable;
  void *VertexPtr;
  int VertexStride;
  int VertexSize;
  GLenum VertexType;

  GLboolean NormalEnable;
  void *NormalPtr;
  int NormalStride;
  GLenum NormalType;

  GLboolean ColorEnable;
  void *ColorPtr;
  int ColorStride;
  int ColorSize;
  GLenum ColorType;

  GLboolean TexEnable;
  void *TexPtr;
  int TexStride;
  int TexSize;
  GLenum TexType;

  GLboolean EdgeFlagEnable;
  void *EdgeFlagPtr;
  int EdgeFlagStride;

  // Raster Position
  GLfloat RasObj[4];
  glLocalVertex RasVtx;
  GLboolean RasValid;

  // Matrix state
  GLenum MatMode;

  GLfloat CurModelView[16];
  GLfloat CurInvTransp[16];
  GLfloat ModelView[32][16];
  int ModelViewPos;
  GLboolean InvTranspDirty;

  GLfloat CurProjMat[16];
  GLfloat ProjMat[2][16];
  int ProjMatPos;

  GLfloat CurComposite[16];
  GLboolean CompositeDirty;
  GLboolean NeedEye;
  GLboolean NeedEyeDirty;

  GLfloat CurTexMat[16];
  GLfloat TexMat[2][16];
  int TexMatPos;
  GLboolean TexMatrixIdentity;

  // Model Clip State
  GLboolean ClipEna[6];
  GLfloat ClipDef[6][4];
  GLfloat ClipXform[6][4];

  // Viewport
  GLint ViewX;
  GLint ViewY;
  GLsizei ViewWidth;
  GLsizei ViewHeight;
  GLfloat xScale;
  GLfloat xOffset;
  GLfloat yScale;
  GLfloat yOffset;
  GLfloat zScale;
  GLfloat zOffset;

  GLfloat zNear;
  GLfloat zFar;

  // Lighting
  glLight Light[8];
  GLboolean LightEna[8];

  GLfloat LightModelAmbient[4];

  GLfloat FrontAmbient[4];
  GLfloat FrontDiffuse[4];
  GLfloat FrontSpecular[4];
  GLfloat FrontEmission[4];
  GLfloat FrontShininess;

  GLfloat BackAmbient[4];
  GLfloat BackDiffuse[4];
  GLfloat BackSpecular[4];
  GLfloat BackEmission[4];
  GLfloat BackShininess;

  GLenum ColorMaterialMode;
  GLenum ColorMaterialFace;

  GLenum ShadeModel;

  // Lighting Validation
  GLboolean LightDirty;
  GLenum EmissionTypeFront;
  GLenum EmissionTypeBack;
  GLenum AmbientTypeFront;
  GLenum AmbientTypeBack;
  GLenum DiffuseTypeFront;
  GLenum DiffuseTypeBack;
  GLenum SpecularTypeFront;
  GLenum SpecularTypeBack;

  GLfloat FrontAmbCache[3];
  GLfloat BackAmbCache[3];

  // Scissor
  GLint minx;
  GLint miny;
  GLint maxx;
  GLint maxy;
  
  // Zbuffer
  GLenum DepthFunc;
  GrCmpFnc_t gDepthFunc;
  FxI16 DepthBiasLevel;

  // Stencil
  GLenum StencilZPass;
  GLenum StencilZFail;
  GLenum StencilFail;
  GLenum StencilFunc;
  GLuint StencilFuncMask;
  GLint StencilRef;
  GLuint StencilMask;

  // Poly State
  GLenum PolygonFrontMode;
  GLenum PolygonBackMode;
  GrTexInfo PolyStippleTex;
  unsigned char PolyStippleData[32*32];
  GLboolean PolyStippleDirty;

  // Alpha Test
  GLenum AlphaFunc;
  GrCmpFnc_t gAlphaFunc;
  GrAlpha_t AlphaRef;

  // Blend
  GLenum SrcBlend;
  GrAlphaBlendFnc_t gSrcBlend;
  GLenum DstBlend;
  GrAlphaBlendFnc_t gDstBlend;

  // Buffer State
  GLenum DrawBuffer;
  GLenum RenderMode;

  // Fog State
  GrFog_t FogTable[GR_FOG_TABLE_SIZE];
  GLenum FogMode;
  GLfloat FogDensity;
  GLfloat FogStart;
  GLfloat FogEnd;
  GrColor_t FogColor;

  // Line State
  GLint LineStippleFactor;
  GLushort LinePattern;
  GLfloat LineWidth;
  GrTexInfo LineStippleTex;
  unsigned char LineStippleData[16*2];
  GLboolean LineStippleDirty;
  float LinePos;

  // Point State
  GLfloat PointWidth;

  GLboolean ContextDirty;

  // Attribute Stack
  glAttrib AttribStack[16];
  GLint AttribDepth;

  // Pixel State
  GLenum ReadBuffer;
  GLfloat PixelMaps[10][256];
  GLint PixelMapSize[10]; 

  GLfloat PixelZoomX;
  GLfloat PixelZoomY;

  GLboolean PackSwapBytes;
  GLboolean PackLsbFirst;
  GLint PackRowLength;
  GLint PackSkipPixels;
  GLint PackSkipRows;
  GLint PackAlignment;

  GLboolean UnPackSwapBytes;
  GLboolean UnPackLsbFirst;
  GLint UnPackRowLength;
  GLint UnPackSkipPixels;
  GLint UnPackSkipRows;
  GLint UnPackAlignment;

  GLboolean PixelMapColor;
  GLboolean PixelMapStencil;
  GLint PixelIndexShift;
  GLint PixelIndexOffset;

  GLfloat PixelRedScale;
  GLfloat PixelRedBias;
  GLfloat PixelGreenScale;
  GLfloat PixelGreenBias;
  GLfloat PixelBlueScale;
  GLfloat PixelBlueBias;
  GLfloat PixelAlphaScale;
  GLfloat PixelAlphaBias;
  GLfloat PixelDepthScale;
  GLfloat PixelDepthBias;

  // Texture state
  pglTexture Tex1DPtr;
  GLuint Tex1DName;
  pglTexture Tex1DDefPtr;
  pglTexture Tex2DPtr;
  GLuint Tex2DName;
  pglTexture Tex2DDefPtr;
  pglTexture TexFastList[NUM_FAST_TEX];
  GLint TexMode;
  GLfloat TexEnvColor[4];
  GLboolean TexDirty;
  pglTexBlock TexHead;
  pglTexture CacheHead;
  int RemCacheSize;
  unsigned int MinCacheAddr;
  unsigned int MaxCacheAddr;

  // Accumulation buffer state
  GLfloat *AccumBuf;

  // Display list state
  pglListBlock ListHead;
  GLboolean Listing;
  GLenum Execute;
  unsigned int *BuildListStart;
  unsigned int *BuildDataStart;
  boolean BuildProblem;
  GLuint BuildName;
  unsigned int *BuildCurBlock;
  unsigned int *BuildCur;
  unsigned int BuildLeft;

  void *TexBuildData;
  unsigned char *TexBuildLevel[9];
  GrTextureFormat_t TexBuildFormat;

  int ExecLevel;
  GLuint ListBase;
  unsigned int *ExecCurBlock[64];
  unsigned int *ExecCur[64];
  unsigned int ExecLeft[64];

  // Select State
  GLuint *SelectBuffer;
  GLuint *CurSelectBuffer;
  GLint SelectSize;
  GLint CurSelectSize;
  GLuint NameStack[64];
  int SelectDepth;
  int SelectNumHits;

  // Feedback State
  GLfloat *FeedbackBuffer;
  GLfloat *CurFeedbackBuffer;
  GLint FeedbackSize;
  GLint CurFeedbackSize;
  int FeedbackNumHits;
  GLenum FeedbackType;

  // Enable/disable state
  GLboolean DepthBuffer;
  GLboolean StencilTest;
  GLboolean Normalize;
  GLboolean Lighting;
  GLboolean Local_Viewer;
  GLboolean Two_Sided;
  GLboolean ColorMaterial;
  GLboolean CullEnable;
  GLboolean ScissorEnable;
  GLboolean PointAA;
  GLboolean LineAA;
  GLboolean PolyAA;
  GLboolean AlphaTest;
  GLboolean Blend;
  GLboolean Tex1D;
  GLboolean Tex2D;
  GLboolean PolyOffset;
  GLboolean LineOffset;
  GLboolean PointOffset;
  GLboolean Fog;
  GLboolean AutoNormal;
  GLboolean LineStipple;
  GLboolean PolyStipple;
  GLboolean Dither;

  // Mark Delta's
  GLboolean FullScreen;
  GLint WindowWidth;
  GLint WindowHeight;
} glContext, *pglContext;

enum {
  PA_POINT1,
  PA_LINE1,
  PA_LINE2,
  PA_LINE_STRIP1,
  PA_LINE_STRIP2,
  PA_LINE_STRIP3,
  PA_LINE_STRIP4,
  PA_LINE_LOOP1,
  PA_LINE_LOOP2,
  PA_LINE_LOOP3,
  PA_LINE_LOOP4,
  PA_TRIANGLES1,
  PA_TRIANGLES2,
  PA_TRIANGLES3,
  PA_TRIANGLE_STRIP1,
  PA_TRIANGLE_STRIP2,
  PA_TRIANGLE_STRIP3,
  PA_TRIANGLE_STRIP4,
  PA_TRIANGLE_STRIP5,
  PA_TRIANGLE_STRIP6,
  PA_TRIANGLE_STRIP7,
  PA_TRIANGLE_STRIP8,
  PA_TRIANGLE_STRIP9,
  PA_TRIANGLE_FAN1,
  PA_TRIANGLE_FAN2,
  PA_TRIANGLE_FAN3,
  PA_TRIANGLE_FAN4,
  PA_TRIANGLE_FAN5,
  PA_QUADS1,
  PA_QUADS2,
  PA_QUADS3,
  PA_QUADS4,
  PA_QUAD_STRIP1,
  PA_QUAD_STRIP2,
  PA_QUAD_STRIP3,
  PA_QUAD_STRIP4,
  PA_QUAD_STRIP5,
  PA_QUAD_STRIP6,
  PA_QUAD_STRIP7,
  PA_QUAD_STRIP8,
  PA_POLYGON1,
  PA_POLYGON2,
  PA_POLYGON3,
  PA_POLYGON4,
  PA_POLYGON5,
  PA_FLUSH
};

enum {
  OP_NORMAL_COORD,
  OP_VERTEX_COORD,
  OP_COLOR_COORD,
  OP_TEXTURE_COORD,
  OP_EDGE_COORD,
  OP_BEGIN,
  OP_END,
  OP_POINTSIZE,    // UNIMP
  OP_LINEWIDTH,    // UNIMP
  OP_LINESTIPPLE,  // UNIMP
  OP_FRONTFACE,
  OP_CULLFACE,
  OP_SHADEMODEL,
  OP_FRONTMODE,
  OP_BACKMODE,
  OP_POLYOFFSET,
  OP_POLYSTIPPLE,
  OP_ALPHAFUNC,
  OP_BLENDFUNC,
  OP_DEPTHFUNC,
  OP_CLIPPLANE,
  OP_FOGDENISTY,
  OP_FOGSTART,
  OP_FOGEND,
  OP_FOGCOLOR,
  OP_FOGMODE,
  OP_DEPTHRANGE,
  OP_VIEWPORT,
  OP_SCISSOR,
  OP_MATRIXMODE,
  OP_LOADIDENTITY,
  OP_LOADMATRIX,
  OP_MULTMATRIX,
  OP_PUSHMATRIX,
  OP_POPMATRIX,
  OP_SETBOOL,
  OP_CLEARDEPTH,
  OP_CLEARCOLOR,
  OP_CLEARACCUM,
  OP_DEPTHMASK,
  OP_COLORMASK,
  OP_CLEAR,
  OP_STENCILOP,
  OP_STENCILMASK,
  OP_STENCILFUNC,
  OP_RASTERPOS,
  OP_LOCALVIEWER,
  OP_TWOSIDE,
  OP_LIGHTMODELAMBIENT,
  OP_SPOTEXPONENT,
  OP_SPOTCUTOFF,
  OP_CONSTANTATTENUATION,
  OP_LINEARATTENUATION,
  OP_QUADRATICATTENUATION,
  OP_AMBIENT,
  OP_DIFFUSE,
  OP_SPECULAR,
  OP_POSITION,
  OP_SPOTDIRECTION,
  OP_FRONTSHININESS,
  OP_FRONTAMBIENT,
  OP_FRONTDIFFUSE,
  OP_FRONTSPECULAR,
  OP_FRONTEMISSION,
  OP_BACKSHININESS,
  OP_BACKAMBIENT,
  OP_BACKDIFFUSE,
  OP_BACKSPECULAR,
  OP_BACKEMISSION,
  OP_COLORMATERIAL,
  OP_INITNAMES,
  OP_LOADNAME,
  OP_POPNAME,
  OP_PUSHNAME,
  OP_PASSTHRU,
  OP_BIND_1D,
  OP_BIND_2D,
  OP_TEXTURE,
  OP_TEXTUREMIP,
  OP_TEXTUREENVCOLOR,
  OP_TEXTUREENVMODE,
  OP_TEXTUREPRIORITY,
  OP_TEXTUREBORDER,
  OP_TEXTURESWRAP,
  OP_TEXTURETWRAP,
  OP_TEXTUREMIN,
  OP_TEXTUREMAX,
  OP_BITMAP,
  OP_DRAWPIXEL,
  OP_PIXELMAP,
  OP_PIXELZOOM,
  OP_PACKSWAPBYTES,
  OP_PACKLSBFIRST,
  OP_PACKROWLENGTH,
  OP_PACKSKIPPIXELS,
  OP_PACKSKIPROWS,
  OP_PACKALIGNMENT,
  OP_UNPACKSWAPBYTES,
  OP_UNPACKLSBFIRST,
  OP_UNPACKROWLENGTH,
  OP_UNPACKSKIPPIXELS,
  OP_UNPACKSKIPROWS,
  OP_UNPACKALIGNMENT,
  OP_MAPCOLOR,
  OP_MAPSTENCIL,
  OP_INDEXSHIFT,
  OP_INDEXOFFSET,
  OP_REDSCALE,
  OP_REDBIAS,
  OP_GREENSCALE,
  OP_GREENBIAS,
  OP_BLUESCALE,
  OP_BLUEBIAS,
  OP_ALPHASCALE,
  OP_ALPHABIAS,
  OP_DEPTHSCALE,
  OP_DEPTHBIAS,
  OP_PUSHATTRIB,
  OP_POPATTRIB,
  OP_ACCUM,
  OP_ACCUMLOAD,
  OP_ACCUMMULT,
  OP_ACCUMADD,
  OP_ACCUMRETURN,
  OP_LISTBASE,
  OP_CALLLIST,
  OP_CALLLIST_OFFSET,
  OP_NEXT_BLOCK,
  OP_END_LIST
};

extern pglContext pglCurContext;
extern int curPixelFormat;
extern void *curWinProc;

void glIntIdentity(GLfloat *mat);
void glIntTranspose(GLfloat *mat);
void glIntMatCopy(GLfloat *src, GLfloat *dest);
void glIntMatMult(GLfloat *new, GLfloat *old);
void glIntInverse(GLfloat *mat);
void glIntNormalize(GLfloat *mat);
void glIntXform(GLfloat *coord, GLfloat *mat, GLfloat *rslt);
void glIntXform3(GLfloat *coord, GLfloat *mat, GLfloat *rslt);
void glIntValidateInvTransp(void);
void glIntValidateComposite(void);
void glIntValidateNeedEye(void);
void glIntValidateLighting(void);
void glIntClose(void);
float glIntCCW(glLocalVertex *Vtx1, glLocalVertex *Vtx2, glLocalVertex *Vtx3);
void glIntFlat(glLocalVertex *Vtx);
void glIntLight(glLocalVertex *Vtx);
void glIntVertex(void);
void glIntRasterPos(void);
void glIntInitContext(pglContext);
void glIntInitLighting(pglContext);
void glIntInitTexture(pglContext);
void glIntInitTextureBlock(pglTexture);
void glIntInitFog(GrFog_t *, GLenum, GLfloat, GLfloat, GLfloat);
void glIntInitAccum(pglContext);
void glIntValidateTexture(void);
void glIntBindTexture(GLenum, GLuint);
void glIntValidateContext(void);
void glIntWritePixel(unsigned char **,unsigned char **,
                     GLenum,GLenum,GrTextureFormat_t);
void glIntReadPixel(unsigned char **,unsigned char **,
                    GLenum,GLenum,GrTextureFormat_t);
void glIntDrawPoint(glLocalVertex *);
void glIntDrawLine(glLocalVertex *, glLocalVertex *);
void glIntDrawTriangle(glLocalVertex *, glLocalVertex *, glLocalVertex *);
void glIntProject(glLocalVertex *);
void glIntOutCodes(glLocalVertex *);
void glIntInterp(glLocalVertex *, glLocalVertex *, glLocalVertex *, int);
void glIntClipDrawLine(glLocalVertex *, glLocalVertex *);
void glIntClipDrawTriangle(glLocalVertex *, glLocalVertex *, glLocalVertex *);
void glIntGet(GLenum, void *, int);
void glIntRead(void *, void *, int, int);
void glIntReadC(void *, int, int);
void glIntGetMaterial(GLenum, GLenum, void *, int);
void glIntGetLight(GLenum, GLenum, void *, int);
void glIntGetTexEnv(GLenum, GLenum, void *, int);
void glIntLoadArray(void *, void *,  void *, void *, void *);
void glIntExecBlock(int);
void glIntIntToList(int);
int glIntIntFromList(void);
void glIntFloatToList(float);
float glIntFloatFromList(void);
void glIntMakeRoom(unsigned int);
void glIntWriteSelect(GLfloat, GLfloat);
void glIntWriteSelectInt(unsigned int);
void glIntWriteFeedback(glLocalVertex *);
void glIntWriteFeedbackFloat(GLfloat);
void glIntCopyAttribToContext(pglAttrib);
void glIntCopyContextToAttrib(pglAttrib, GLbitfield);
void gl_vid_shutdown( void );
int  gl_vid_init( void );


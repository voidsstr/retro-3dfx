
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <string.h>

#include <Memory.h>
#include <QuickDraw.h>
#include <RAVE.h>
#include <RaveSystem.h>
#include <TextUtils.h>
#include <Timer.h>
#include "r3Tweaks.h"

#include <glide.h>



struct grvert 
{
	float x, y, ooz;
	float oow;
	FxU32 argb;			// 16
	float sow0;			// 20
	float tow0;
	float q0;			// 28
	float sow1;			// 32
	float tow1;
	float q1;			// 40
};

typedef struct grvert GrVertex;




#define RvAbs(a) ((a) < 0.0 ? -(a) : (a))

#include <gl.h>
#include "r3Glide.h"
#include "r3State.h"
#include "r3ZSort.h"


#include <file_io.h>

#include <cassert>

#define AA_SUPPORT

// Apple should add this to QD3DAcceleration.h, but they prob won't.
// G. Stahl of Apple said that 17 seemed OK.
#define kQAVendor_3Dfx			 17		// was 8 for SST-1, but that belongs to kQAVendor_D3DAdaptor
#define kQAEngine_3DfxSST1	 		0	// not used; voodoo2
#define kQAEngine_3DfxVoodooVSA100	 	1

#define TNSL_WINDOW				   0
#define TNSL_WINDOW_DUALHEAD       0

#define TNSL_NUMMODES				12

#define TNSL_NONE					0
#define TNSL_GOURAUD				1
#define TNSL_TEXTURE				2

#define TNSL_UOUTOFRANGE		1
#define TNSL_VOUTOFRANGE		2
#define TNSL_WOUTOFRANGE		4
#define TNSL_VALID					8

#define TNSL_MAXUOVERW	    127.0f
#define TNSL_MININVW	    	(1.0f / 16383.0f)

#define TNSL_NONE					0
#define TNSL_GOURAUD				1
#define TNSL_TEXTURE				2
#define TNSL_BITMAP					3

#define TNSL_GLIDE_MIPMAPNONE		  0
#define TNSL_GLIDE_MIPMAPNEAREST	1
#define TNSL_GLIDE_MIPMAPDITHERED	2

#define TNSL_GLIDE_PITCH					2048
#define TNSL_PIXEL_FORMAT_COUNT		7


#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define max(a,b)	(((a) > (b)) ? (a) : (b))

#define bswap(a)	(((a) << 24) | \
(((a) << 8) & 0xFF0000) | \
(((a) >> 8) & 0xFF00) | \
((a) >> 24))


#if TNSL_DEBUG
#define DbgMsg(s) SysBreakStr("\p3Dfx RAVE: " s);
#define DbgErr(s) { if(err) SysBreakStr("\p3Dfx RAVE: failed " s); }

#if TNSL_DEBUG_VERTS
#define DbgRaveVertexGouraud(v) \
{ \
  unsigned char c[256]; \
  sprintf((char*) c, "%ld %ld %ld %ld %ld %ld %ld %ld\n", \
          (long) (v)->x, (long) (v)->y, \
          (long) ((v)->z * 65535.0f), (long) (v)->invW, \
          (long) ((v)->r * 255.0f), (long) ((v)->g * 255.0f), \
          (long) ((v)->b * 255.0f), (long) ((v)->a * 255.0f)); \
  SysBreakStr(c); \
}

#define DbgRaveVertexTexture(rv, t) \
{ \
  char c[150]; \
  if (Button() )	\
  {				\
		/*				  x     y     z*     r      g     b     a   kd_r  kd_g  kd_b   u/w*   v/w*  */	\
  sprintf((char*)(c+1), "%5.0f %5.0f %9.1f  %6.1f %6.1f %6.1f %6.1f %6.1f %6.1f %6.1f %10.3f %10.3f \n", 	\
		(rv)->x,  (rv)->y, 												\
		((rv)->z * 65536.0f), 											\
		((rv)->r * 255.9f),  ((rv)->g * 255.9f), 						\
		((rv)->b * 255.9f),  ((rv)->a * 255.9f), 						\
		((rv)->kd_r * 255.9f), ((rv)->kd_g * 255.9f), ((rv)->kd_b * 255.9f), \
		((rv)->uOverW * (t)->ratioX),  ((rv)->vOverW * (t)->ratioY) ); 		\
	c[0] = strlen(c+1);													\
  SysBreakStr( (ConstStr255Param)c); \
  }									\
}

#define DbgGlideVertexTexture(grv) \
{ \
  char c[150]; 																			\
  if (Button()	)																		\
  {																						\
  /*				     x     y   ooz    pargb  sow0   tow0    q0      oow */			\
sprintf((char*)(c+1), "%5.0f %5.0f %9.1f 0x%08X  %10.3f %10.3f %10.3f %10.3f\n", 	\
					grv.x, grv.y, grv.ooz,  grv.argb,								\
					 grv.sow0, grv.tow0, grv.q0, grv.oow  );						\
	c[0] = strlen(c+1);																\
  SysBreakStr((ConstStr255Param)c); 												\
  }																					\
}
#else
#define DbgRaveVertexGouraud(v)
#define DbgRaveVertexTexture(rv,t)
#define DbgGlideVertexTexture(grv)
#endif

#ifdef __MWERKS__
#pragma global_optimizer off
#pragma scheduling			 off
#pragma traceback				 on
#endif /* __MWERKS__ */

#else /* !TNSL_DEBUG */
#define DbgMsg(s)
#define DbgErr(s)
#define DbgRaveVertexGouraud(v)
#define DbgRaveVertexTexture(rv,t)
#define DbgGlideVertexTexture(grv)

#ifdef __MWERKS__
//+ RES I want to control these things in the project preferences.
//+ #pragma global_optimizer	 on
//+ #pragma optimization_level 4
//+ #pragma peephole					 on
//+ #pragma scheduling				 604
//+ #pragma traceback					 off
//+ #pragma side_effects			 off
#endif /* __MWERKS__ */

#endif /* !TNSL_DEBUG */


#define DebugVertexGouraud(v) \
  fprintf(gRvEngInfo.pFileLog, "%ld %ld ",  (long) (v)->x, (long) (v)->y); \
  fprintf(gRvEngInfo.pFileLog, "%ld %2.8f ",  (long) ((v)->z * 65535.0f), (v)->invW); \
  fprintf(gRvEngInfo.pFileLog, "%ld %ld ",  (long) ((v)->r * 255.0f), (long) ((v)->g * 255.0f)); \
  fprintf(gRvEngInfo.pFileLog, "%ld %ld\n", (long) ((v)->b * 255.0f), (long) ((v)->a * 255.0f)); \
  fflush(gRvEngInfo.pFileLog);
#define DebugVertexTexture(v) \
  fprintf(gRvEngInfo.pFileLog, "X = %7ld   Y = %7ld   ",  (long) (v)->x, (long) (v)->y); \
  fprintf(gRvEngInfo.pFileLog, "Z = %7.8f   1/W = %7.8f   ",  ((v)->z * 65536.0f), (v)->invW); \
  fprintf(gRvEngInfo.pFileLog, "U/W = %7.8f   V/W = %7.8f\n", (v)->uOverW, (v)->vOverW); \
  fflush(gRvEngInfo.pFileLog);
#define DebugVertexTexCoord(n,t) \
  fprintf(gRvEngInfo.pFileLog, "%3ld %3ld   ", (long) (n)->x, (long) (n)->y); \
  fprintf(gRvEngInfo.pFileLog, "%15.8f %15.8f %15.8f   ", (n)->invW, (n)->uOverW, (n)->vOverW); \
  fprintf(gRvEngInfo.pFileLog, "%15.8f %15.8f %15.8f\n", \
          1.0f / ((n)->invW), (n)->uOverW / (n)->invW, (n)->vOverW / (n)->invW); \
  fflush(gRvEngInfo.pFileLog);
#define DebugMsg(s)	\
  fprintf(gRvEngInfo.pFileLog, "%s", s); \
  fflush(gRvEngInfo.pFileLog);
#define DebugNumber(n) \
  fprintf(gRvEngInfo.pFileLog, "%08x ", n); \
  fflush(gRvEngInfo.pFileLog);
	
#ifdef TNSL_LOG

#define DebugStr(s) \
  fprintf(gRvEngInfo.pFileLog, "%s", s); \
  fflush(gRvEngInfo.pFileLog);
#define DebugNum(n) \
  fprintf(gRvEngInfo.pFileLog, "%ld ", n); \
  fflush(gRvEngInfo.pFileLog);
#define DebugHex(n) \
  fprintf(gRvEngInfo.pFileLog, "0x%08X ", n); \
  fflush(gRvEngInfo.pFileLog);
#define Debug4Num(a,b,c,d) \
  fprintf(gRvEngInfo.pFileLog, "%ld %ld %ld %ld ", a, b, c, d); \
  fflush(gRvEngInfo.pFileLog);
#define Debug6Num(a,b,c,d,e,f) \
  fprintf(gRvEngInfo.pFileLog, "%ld %ld %ld %ld %ld %ld\n", a, b, c, d, e, f); \
  fflush(gRvEngInfo.pFileLog);
#define DebugFloat(f)	\
  fprintf(gRvEngInfo.pFileLog, "%f ", (f)); \
  fflush(gRvEngInfo.pFileLog);
#else
#define DebugStr(s)
#define DebugNum(n)
#define DebugHex(n)
#define Debug4Num(a,b,c,d)
#define Debug6Num(a,b,c,d,e,f)
#define DebugFloat(f)
#endif

#ifdef TNSL_TIMER
#define DrvTimerStart() \
  gRvEngInfo.drvTimer.absDrvStart = UpTime();
#define DrvTimerEnd()	\
  gRvEngInfo.drvTimer.absDrvEnd = UpTime(); \
  gRvEngInfo.drvTimer.absDrvEnd = SubAbsoluteFromAbsolute(gRvEngInfo.drvTimer.absDrvEnd, \
                                                        gRvEngInfo.drvTimer.absDrvStart);	\
  gRvEngInfo.drvTimer.absDrv    = AddAbsoluteToAbsolute(gRvEngInfo.drvTimer.absDrv, \
                                                      gRvEngInfo.drvTimer.absDrvEnd);
#define AppTimerStart()	\
  gRvEngInfo.drvTimer.absAppStart = UpTime();
#define AppTimerEnd()	\
  gRvEngInfo.drvTimer.absAppEnd = UpTime(); \
  gRvEngInfo.drvTimer.absAppEnd = SubAbsoluteFromAbsolute(gRvEngInfo.drvTimer.absAppEnd, \
                                                        gRvEngInfo.drvTimer.absAppStart);	\
  gRvEngInfo.drvTimer.absApp    = AddAbsoluteToAbsolute(gRvEngInfo.drvTimer.absApp, \
                                                      gRvEngInfo.drvTimer.absAppEnd);
#define DevTimerStart()	\
  gRvEngInfo.drvTimer.absDevStart = UpTime();
#define DevTimerEnd()	\
  gRvEngInfo.drvTimer.absDevEnd = UpTime(); \
  gRvEngInfo.drvTimer.absDevEnd = SubAbsoluteFromAbsolute(gRvEngInfo.drvTimer.absDevEnd, \
                                                        gRvEngInfo.drvTimer.absDevStart);	\
  gRvEngInfo.drvTimer.absDev    = AddAbsoluteToAbsolute(gRvEngInfo.drvTimer.absDev, \
                                                      gRvEngInfo.drvTimer.absDevEnd);
#define TexTimerStart()	\
  gRvEngInfo.drvTimer.absTexStart = UpTime();
#define TexTimerEnd()	\
  gRvEngInfo.drvTimer.absTexEnd = UpTime(); \
  gRvEngInfo.drvTimer.absTexEnd = SubAbsoluteFromAbsolute(gRvEngInfo.drvTimer.absTexEnd, \
                                                        gRvEngInfo.drvTimer.absTexStart);	\
  gRvEngInfo.drvTimer.absTex    = AddAbsoluteToAbsolute(gRvEngInfo.drvTimer.absTex, \
                                                      gRvEngInfo.drvTimer.absTexEnd);
#else
#define DrvTimerStart()
#define DrvTimerEnd()
#define AppTimerStart()
#define AppTimerEnd()
#define DevTimerStart()
#define DevTimerEnd()
#define TexTimerStart()
#define TexTimerEnd()
#endif

/*
 * rendering state change flags; used for deferring register writes
 */
#define STATE_NOP					(1 << 0)
#define	STATE_PT_WIDTH				(1 << 1)
#define STATE_DEPTH_BG				(1 << 2)
#define	STATE_BLEND					(1 << 3)
#define STATE_TEXTURE				(1 << 4)
#define STATE_COMPOSITE				(1 << 5)
#define STATE_FOG					(1 << 6)
#define STATE_Z_FUNC				(1 << 7)
#define STATE_DITHER_MODE			(1 << 8)
#define STATE_ALPHA_TST				(1 << 9)
#define STATE_ALPHA_TST_FUNC		(1 << 10)
#define STATE_ALPHA_TST_REF			(1 << 11)
#define STATE_ROP2					(1 << 12)
#define STATE_GL_SCISSOR			(1 << 13)
#define STATE_RGB_MASK				(1 << 14)
#define STATE_SHADE_MODE			(1 << 15)
#define STATE_GL_BLEND_MODE			(1 << 16)
#define STATE_APPLE_YUV_MODE		(1 << 17)
#define STATE_GL_CULL_MODE			(1 << 18)
#define STATE_LINE_STIPPLE_MODE		(1 << 19)	
#define STATE_POLYGON_STIPPLE_MODE	(1 << 20)
#define STATE_DRAW_DESTINATION		(1 << 21)
#define STATE_CHROMAKEY				(1 << 22)
#define STATE_DEPTHBUFFER			(1 << 23)
#define STATE_ALPHASORTING			(1 << 24)
#define STATE_ANTIALIAS				(1 << 25)

/*
 * primary texture state flags
 */
#define	STATE_PRIMARY_TEX			(1 << 0)
#define	STATE_TEX_FILTER			(1 << 1)
#define	STATE_TEX_OP				(1 << 2)
#define STATE_LOD_BIAS				(1 << 3)
#define STATE_TEX_WRAPU				(1 << 4)
#define STATE_TEX_WRAPV				(1 << 5)
#define STATE_TEX_VQ				(1 << 6)
#define STATE_GL_BORDER_COLOR		(1 << 7)
#define STATE_TEX_MIN				(1 << 8)
#define STATE_TEX_MAG				(1 << 9)


/*
 * secondary texture state flags
 */
#define STATE_COMP_TEX				(1 << 0)
#define STATE_COMP_MODE				(1 << 1)
#define STATE_COMP_TEX_FILTER		(1 << 2)
#define STATE_COMP_TEX_OP			(1 << 3)
#define STATE_COMP_TEX_MIN			(1 << 4)
#define STATE_COMP_TEX_MAG			(1 << 5)
#define STATE_COMP_TEX_WRAPU		(1 << 6)
#define STATE_COMP_TEX_WRAPV		(1 << 7)
#define STATE_COMP_GL_BORDER_COLOR	(1 << 8)

/*
 * fog state tFogChangeMask flags
 */
#define STATE_FOG_MODE				(1 << 0)
#define STATE_FOG_COLOR				(1 << 1)
#define STATE_FOG_START				(1 << 2)
#define STATE_FOG_END				(1 << 3)	
#define STATE_FOG_DENSITY			(1 << 4)
#define STATE_FOG_MAX_DEPTH			(1 << 5)

/*
 * For implementing OpenGL texture management
 */


#define GLR_MAX_TEXTURE_LEVEL     12
#define GLR_NUM_TEXTURE_TARGETS   2
#define GLR_NUM_TEXTURE_UNITS     2

#define GLR_TMU0_ACTIVE (1L << GR_TMU0)
#define GLR_TMU1_ACTIVE (1L << GR_TMU1)

#include "r3Texture.h"


struct TQAColorTable {
	TQAColorTableType	pixelType;
	long				transparentIndexFlag; // if true then idx 0 is chromakey.
	UInt32				pixelData[ 256 ];
};


typedef struct {
	unsigned long flags;
	float u;
	float v;
	float w;
} TTexCoord;

typedef struct {
	unsigned long fps;
	unsigned long numFrame;
	unsigned long numTriangle;
	unsigned long numVertex;
	unsigned long frameCount;
	unsigned long seconds;
	unsigned long frames;

	unsigned long minAbsoluteTimeDelta;
	unsigned long theAbsoluteTimeToNanosecondNumerator;
	unsigned long theAbsoluteTimeToNanosecondDenominator;
	unsigned long theProcessorToAbsoluteTimeNumerator;
	unsigned long theProcessorToAbsoluteTimeDenominator;
	
	AbsoluteTime absTotal;
	AbsoluteTime absStart;
	AbsoluteTime absEnd;
	
	AbsoluteTime absDev;
	AbsoluteTime absDevStart;
	AbsoluteTime absDevEnd;
	AbsoluteTime absDrv;
	AbsoluteTime absDrvStart;
	AbsoluteTime absDrvEnd;
	AbsoluteTime absApp;
	AbsoluteTime absAppStart;
	AbsoluteTime absAppEnd;
	AbsoluteTime absTex;
	AbsoluteTime absTexStart;
	AbsoluteTime absTexEnd;
	
	float perDev1;
	float perDrv1;
	float perApp1;
	float perTex1;
	float perDev2;
	float perDrv2;
	float perApp2;
	float perTex2;
} TTimer;

typedef struct enginfo {					// offsets
	UInt32 					debug;
	UInt32					boardID;
	UInt32 					numDrawContexts;			// 
	TQADrawContext			*currentDrawContext;
	
	FxI32					sizeOfGlideState;

	TQATexture 				*textureList;    	// linked list off all textures attached to this rave engine	32
	SInt32 					textureCount;
	SInt32 					totalTextureSize;

	UInt32					numBoards;					// 
	FxI32 					totalVRAMcard;
	FxI32					VRAMusedForNonTextures;
	FxI32 					surface_size;
	GrSurface_t				texture_surface;	// shared by all contexts of a board

	TQATexture 				*first_resident_texture;     // first texture in vram
	TQATexture 				*latest_resident_texture;    // most recently allocated texture

	
#ifdef TNSL_LOG	
	FILE* pFileLog;
#endif
	
#ifdef TNSL_TIMER	
	TTimer drvTimer;
#endif
};

typedef struct enginfo TRvInfo;

extern void (*BltMipMap[TNSL_PIXEL_FORMAT_COUNT])(void*, TQAImage*, UInt32);

#if TNSL_DEBUG_MEM
void MemoryCheck(void);
#else
#define MemoryCheck()
#endif
Ptr AllocPtr(Size s);
void FreePtr(Ptr p);


typedef union tnslTState {
	UInt32			i;
	Float32			f;
	const void		*p;
	} tnslTState;


typedef float(*TFogFunc)(float invW, float a, struct TQADrawPrivate * dp);

#define kRaveMaxTag				150		// engine-specific minimum is 1000, but we won't have any.


typedef struct  TQADrawPrivate {
	TQADrawContext		*parent;
	UInt32				ctxID;
	UInt32       		special;
	UInt32				bFullScreen;		//+ RES
	UInt32				bChooseNewTriFcts;	//+

	//+ state variables, organized by Rave.h tags
	tnslTState			tState[ kRaveMaxTag ];
	//+ used to accumulate change flags, so that registers can be written at one time
	UInt32				tChangeMask;
	UInt32				tTex1ChangeMask;
	UInt32				tTex2ChangeMask;
	UInt32				tFogChangeMask;
	Boolean				atiFog;
	TFogFunc			fogFunc;
	UInt32				flags;					//+ Mask of kQAContext_xxx
	long				width;
	long				height;
	TQADevice 			device;
	void				*pBaseAddr;
	FxU32				glideBoardSelectID;		//+

 	// Display buffer info
	TQARect 			deviceRect;
	TQARect				targetRect;				//+ in device coords.
	// TQARect				viewPort;				//+
	CGrafPtr			targetPort;				//+
	long				screenRowBytes;
	UInt32 				screenWidth;			//+ seems redundant
	UInt32 				screenHeight;
	RgnHandle 			maskRgn;					//+ context clipping;
	short				targetPixelBits;		//+ 
	
	// Backbuffer renderPort info
	UInt32				z_bits;					//+ 0, 16, 24, or 32 bits depth
	CGrafPtr			renderPort;				//+
	
	// VRAM
	GrSurface_t			renderSurface;			//+
	GrSurface_t			auxSurface;				//+
	GrContext_t			glideContext;			//+
	GrSurfaceDesc_t		renderSurfaceDesc;		//+
	GrSurfaceDesc_t		auxSurfaceDesc;			//+

	GrFog_t   			*gFogTable;
	
	// render state
	GrCmpFnc_t			glDepthFunc;	
	GrAlphaBlendFnc_t 	rgb_sf;
	GrAlphaBlendFnc_t 	rgb_df;
	UInt32 				currTriType;
	UInt32 				usingVertexAlpha;
	UInt32 				usingTextureAlpha[GLR_NUM_TEXTURE_UNITS];
	// UInt32				currAlphaBits;
	TQAColorTable		*currColorTable;

#if CUSTOM_GLIDE_STATE_DEF
	
	GrCombineFunction_t	currAlphaCombine_function; 
	GrCombineFactor_t	currAlphaCombine_factor;
	GrCombineLocal_t	currAlphaCombine_local; 
	GrCombineOther_t	currAlphaCombine_other;
	
	GrCombineFunction_t currColorCombineFunction; 
	GrCombineFactor_t 	currColorCombineFactor;
	GrCombineLocal_t 	currColorCombineLocal; 
	GrCombineOther_t 	currColorCombineOther;

	GrCombineFunction_t currTexCombineRgb_function;
	GrCombineFactor_t 	currTexCombineRgb_factor; 
	GrCombineFunction_t currTexCombineAlpha_function;
	GrCombineFactor_t 	currTexCombineAlpha_factor;
#endif	
	// current textures & colortable
	TQATexture* 		currBaseTexture;
	TQATexture* 		lastBaseTextureProcessed;
	TQATexture* 		currMultiTexture;
	TQAColorTable* 		colorTable;
	GrColor_t 			currChromakeyValue;
	
	// workspace vars
	UInt32       	nVerticesGouraud;
	UInt32       	countGouraud;
	TQAVGouraud* 	gouraudVertexList;

	UInt32       	nVerticesTexture;
	UInt32       	countTexture;
	TQAVTexture* 	textureVertexList;
	GrVertex*    	grVerticesTexture;
	
	// Cached transparent triangles.
	Boolean			bStateInCacheChanged;
	
	#if CUSTOM_GLIDE_STATE_DEF
	TRaveRenderGlideState *pCurrAlphaGlideState;
	TRaveRenderGlideState *pAlphaGlideStates;
	#else
	void *pCurrAlphaGlideState;
	void *pAlphaGlideStates;
	#endif
	
	UInt32			bSortingAlphaTri;
	UInt32			maxCachedTri;
	UInt32			maxCachedStates;
	UInt32			currStateIndex;
	Boolean			bTriCacheFull;
	UInt32       	idxAlphaTri;		// where we are in the cached triangle list
	UInt32			idxAlphaTriStatesList; // where we are in the list of state changes
	UInt32       	countAlphaTri;		// running total
	CacheTriangle   cachedTri[kMaxAlphaTriInCache];
	CacheIndex		cacheIndex[kMaxAlphaTriInCache];
	
	UInt32			nMultiTexParams;
	TQAVMultiTexture*	multiTexParams;
	
	/* callbacks */
	TQANoticeMethod 	completionCallBack[kQAMethod_NumSelectors]; 	// num selectors = 5
	void*           	callBackRefCon[kQAMethod_NumSelectors];			// total size = 112
	UInt32			surfaceLost;

};


/************************************************************************************************
 * The TQAStorePrivate datatypes. This is just a placeholder; these are
 * engine-specific. A single TTtStorePrivate global instance is used to hold
 * information on all bitmaps and textures allocated by the engine.
 ***********************************************************************************************/

OSErr 
RvInitialize(void);

OSErr 
RvTerminate(void);

TQAError 
RvEngineGetMethod(TQAEngineMethodTag methodTag,
                    TQAEngineMethod*   method);

TQAError 
RvDrawPrivateNew(TQADrawContext*  newDrawContext,
                   const TQADevice* device,
                   const TQARect*   rect,
                   const TQAClip*   clip,
                   unsigned long    flags);

void 
RvDrawPrivateDelete(TQADrawPrivate* drawPrivate);

TQAError 
RvEngineDeviceCheck(const TQADevice* device);

TQAError 
RvEngineGestalt(TQAGestaltSelector selector,
                  void*              response);

TQAError 
RvTextureNew(unsigned long     flags,
               TQAImagePixelType pixelType,
               const TQAImage    images[],
               TQATexture**      newTexture);

TQAError 
RvTextureDetach(TQATexture* texture);

void 
RvTextureDelete(TQATexture* texture);

TQAError 
RvBitmapNew(unsigned long     flags,
              TQAImagePixelType pixelType,
              const TQAImage*   image,
              TQABitmap**        newBitmap);

TQAError 
RvBitmapDetach(TQABitmap* bitmap);

void 
RvBitmapDelete(TQABitmap* bitmap);

#if 0
void 
RvSubmitVerticesGouraud(const TQADrawContext* drawContext,
                          unsigned long         nVertices,
                          const TQAVGouraud*    vertices);
#endif

TQAError 
RvColorTableNew(TQAColorTableType pixelType,
                  void*             pixelData,
                  long              transparentIndexFlag,
                  TQAColorTable**   newTable);

void 
RvColorTableDelete(TQAColorTable* colorTable);

TQAError 
RvTextureBindColorTable(TQATexture*    texture,
                          TQAColorTable* colorTable);

TQAError 
RvBitmapBindColorTable(TQABitmap*     bitmap,
                         TQAColorTable* colorTable);


///////////////////////////
//                       //
//  drawContext methods  //
//                       //
///////////////////////////

void 
RvSetFloat(TQADrawContext* drawContext,
             TQATagFloat     tag,
             float           newValue);

void 
RvSetInt(TQADrawContext* drawContext,
           TQATagInt       tag,
           unsigned long   newValue);

void 
RvSetPtr(TQADrawContext* drawContext,
           TQATagPtr       tag,
           const void*     newValue);

float 
RvGetFloat(const TQADrawContext* drawContext,
             TQATagFloat           tag);

unsigned long 
RvGetInt(const TQADrawContext* drawContext,
           TQATagInt             tag);

void* 
RvGetPtr(const TQADrawContext* drawContext,
           TQATagPtr             tag);

void
RvDrawPoint(const TQADrawContext* drawContext,
              const TQAVGouraud*    v0);

void 
RvDrawLine(const TQADrawContext* drawContext,
             const TQAVGouraud*    v0,
             const TQAVGouraud*    v1);

void 
RvDrawTriGouraud(const TQADrawContext* drawContext,
                   const TQAVGouraud*    v0,
                   const TQAVGouraud*    v1,
                   const TQAVGouraud*    v2,
                   unsigned long         flags);

void 
RvDrawTriTexture(const TQADrawContext* drawContext,
                   const TQAVTexture*    v0,
                   const TQAVTexture*    v1,
                   const TQAVTexture*    v2,
                   unsigned long         flags);

void 
RvDrawVGouraud(const TQADrawContext* drawContext,
                 unsigned long         nVertices,
                 TQAVertexMode         vertexMode,
                 const TQAVGouraud     vertices[],
                 const unsigned long   flags[]);

void 
RvDrawVTexture(const TQADrawContext* drawContext,
                 unsigned long         nVertices,
                 TQAVertexMode         vertexMode,
                 const TQAVTexture     vertices[],
                 const unsigned long   flags[]);

void 
RvDrawBitmap(const TQADrawContext* drawContext,
               const TQAVGouraud*    v,
               TQABitmap*            bitmap);
               
Boolean CacheTriGouraud( 	TQADrawContext *dc,
							GrVertex *v0,
							GrVertex *v1,
							GrVertex *v2 );

Boolean CacheTriTexture( 	TQADrawContext *dc,
							GrVertex *v0,
							GrVertex *v1,
							GrVertex *v2);
void FlushTriCache( TQADrawContext *dc );
void resetPipeline( void );	// redirects tri fct ptrs to "careful" wrappers
void ResetContextStateFlags( TQADrawContext * );	// clean state for new frame
void SetRenderState( TQADrawContext* dp, Boolean bTex); // takes action on change flags
void SetGouraudMode( TQADrawPrivate *dp );
void SetTextureModeAndCheckTexAlpha( TQADrawPrivate *dp );
void SetAndLoadTextureMap( TQADrawPrivate* dp );
void SetTexWrapClamp( TQADrawPrivate* dp );				
void SetBaseTextureFilter( TQADrawPrivate *dp );
void SetMultiTextureFilter( TQADrawPrivate *dp );
void SetTexGlideCombinesBasedOnState( TQADrawPrivate *dp, FxBool );
void SetFog( TQADrawPrivate *dp );
void SetSecond_Texture( TQADrawPrivate *dp );
void SetCompositing( TQADrawPrivate *dp, FxBool which );
void SetRenderStateComposite( TQADrawPrivate *dp, Boolean bTexturing );
void SetSecondTexGlideCombinesBasedOnState( TQADrawPrivate *dp );
void SetGlide_Z( TQADrawPrivate* dp);
void SetGlide_Alpha( TQADrawPrivate* dp);
void SetGlide_Mask( TQADrawPrivate* dp);
void SetGlide_DitherMode( TQADrawPrivate* dp);
void ChooseNewTriFcts( TQADrawContext* dc );
void SetDepthBufferMode( TQADrawPrivate *dp );
void SetOpenGLBlending( TQADrawPrivate *dp );
void SetAlphaSorting( TQADrawPrivate *dp );
UInt32 NextPowerOfTwo( UInt32 v );
FxI32	ConvertRaveAlphaTestFunc( TQADrawPrivate *dp );



/* replaced
void SetGlide_GouraudPremultiply(TQADrawPrivate* dp);
void SetGlide_GouraudInterpolate(TQADrawPrivate* dp);
void SetGlide_TexturePremultiplyNone(TQADrawPrivate* dp);
void SetGlide_TexturePremultiplyModulate(TQADrawPrivate* dp);
void SetGlide_TexturePremultiplyHighlight(TQADrawPrivate* dp);
void SetGlide_TexturePremultiplyDecal(TQADrawPrivate* dp);
void SetGlide_TextureInterpolateNone(TQADrawPrivate* dp);
void SetGlide_TextureInterpolateModulate(TQADrawPrivate* dp);
void SetGlide_TextureInterpolateHighlight(TQADrawPrivate* dp);
void SetGlide_TextureInterpolateDecal(TQADrawPrivate* dp);
void SetGlide_TextureModulateHighlight(TQADrawPrivate* dp);
*/

void 
RvRenderStart(const TQADrawContext* drawContext,
                const TQARect*        dirtyRect,
                const TQADrawContext* initialContext);

TQAError 
RvRenderEnd(const TQADrawContext* drawContext,
              const TQARect*        modifiedRect);

TQAError 
RvRenderAbort(const TQADrawContext* drawContext);

TQAError 
RvFlush(const TQADrawContext* drawContext);

TQAError 
RvSync(const TQADrawContext* drawContext);

void 
RvSubmitVerticesGouraud(const TQADrawContext* drawContext,
                          unsigned long         nVertices,
                          const TQAVGouraud*    vertices);

void 
RvSubmitVerticesTexture(const TQADrawContext* drawContext,
                          unsigned long         nVertices,
                          const TQAVTexture*    vertices);

void 
RvDrawTriMeshGouraud(const TQADrawContext*     drawContext,
                       unsigned long             nTriangles,
                       const TQAIndexedTriangle* triangles);

void 
RvDrawTriMeshTexture(const TQADrawContext*     drawContext,
                       unsigned long             nTriangles,
                       const TQAIndexedTriangle* triangles);

void 
RvDrawTriMeshTextureVertexArray(const TQADrawContext*     drawContext,
                       unsigned long             nTriangles,
                       const TQAIndexedTriangle* triangles);

TQAError 
RvSetNoticeMethod(const TQADrawContext* drawContext,
                    TQAMethodSelector     method,
                    TQANoticeMethod       completionCallBack,
                    void*                 refCon);

TQAError 
RvGetNoticeMethod(const TQADrawContext* drawContext,
                    TQAMethodSelector     method,
                    TQANoticeMethod*      completionCallBack,
                    void**                refCon);

void 
RvSubmitMultiTextureParams (
		const TQADrawContext 		*drawContext, /* Draw context */
		unsigned long 				nParams, /* Number of params */
		const TQAVMultiTexture 		*params); /* params */
		
TQABoolean RvBusy( const TQADrawContext *drawContext );

TQAError RvSwapBuffers( 	const TQADrawContext *drawContext, 
							const TQARect *modifiedRect );

TQAError RvAccessTexture(	TQATexture			*texture,
							long 				mipmapLevel,
							long 				flags,
							TQAPixelBuffer		*buffer );

TQAError RvAccessDrawBuffer(  
	const TQADrawContext	*drawContext,	
	TQAPixelBuffer			*pixelBuffer );
	
TQAError RvAccessDrawBufferEnd(  
	const TQADrawContext	*drawContext,					/* Draw context */
	const TQARect			*dirtyRect  );
	
TQAError RvAccessZBuffer( 
	const TQADrawContext 	*drawContext, 
	TQAZBuffer 				*zBuffer);
	
TQAError RvAccessZBufferEnd(
	const TQADrawContext 	*drawContext, 
	const TQARect 			*dirtyRect);

TQAError RvClearDrawBuffer(	const TQADrawContext *drawContext, 
								const TQARect *rect, 
								const TQADrawContext *initialContext);

TQAError RvClearZBuffer(		const TQADrawContext *drawContext, 
								const TQARect *rect, 
								const TQADrawContext *initialContext);

///////////////////////////
//                       //
//  internal prototypes  //
//                       //
///////////////////////////

// sst1Accel.c
void 
sst1_InstallAccel(void);
void 
sst1_UninstallAccel(void);
/*
Int32 sst1_GetProc_BitBlit    (NQDDrawVars*);
Int32 sst1_GetProc_LineBlit   (NQDDrawVars*);
Int32 sst1_GetProc_PatternBlit(NQDDrawVars*);
Int32 sst1_GetProc_PatRgnBlit (NQDDrawVars*);
Int32 sst1_GetProc_RgnBlit    (NQDDrawVars*);
Int32 sst1_GetProc_ScaleBlit  (NQDDrawVars*);
Int32 sst1_GetProc_SlabBlit   (NQDDrawVars*);
Int32 sst1_FinishProc         (NQDDrawVars*);
*/

// sst1ModeSet.c
UInt32 SetScreenResolution(TQADrawPrivate*, UInt32, UInt32);

// sst1Glide.c
// void SetGlide_Gouraud(TQADrawPrivate*);
// void SetGlide_Texture(TQADrawPrivate*);

// sst1Vertex.c
void ConvertUVWNormalize(TQADrawPrivate*, const TQAVTexture*, const TQAVTexture*, const TQAVTexture*, TTexCoord*, TTexCoord*, TTexCoord*, GrVertex*);
inline void SetVertex_GouraudSingle(TQADrawPrivate*, const TQAVGouraud*, GrVertex*);
inline void SetVertex_GouraudThree (TQADrawPrivate*, const TQAVGouraud*, const TQAVGouraud*, const TQAVGouraud*, GrVertex*);
inline void SetVertex_GouraudMany  (TQADrawPrivate*, const TQAVGouraud*, GrVertex*, unsigned long);
inline void SetVertex_TextureSingle(TQADrawPrivate*, const TQAVTexture*, GrVertex*);
inline void SetVertex_TextureMany  (TQADrawPrivate*, const TQAVTexture*, GrVertex*, TTexCoord*, unsigned long);

// sst1Blts.c
void Blt_16to16(UInt16*, UInt16*, UInt32, UInt32, UInt32, UInt32, UInt32);
void Blt_32to32(UInt32*, UInt32*, UInt32, UInt32, UInt32, UInt32, UInt32);
void Blt_8to32 (UInt32*, UInt8*, UInt32*, UInt32, UInt32, UInt32, UInt32, UInt32);
void Blt_4to32 (UInt32*, UInt8*, UInt32*, UInt32, UInt32, UInt32, UInt32, UInt32);

void BltMipMap_16ARGBto16ARGB(void*, TQAImage*, UInt32);
void BltMipMap_32RGBto16RGB  (void*, TQAImage*, UInt32);
void BltMipMap_32ARGBto16ARGB(void*, TQAImage*, UInt32);
void BltMipMap_CL8toCL8      (void*, TQAImage*, UInt32);
void BltMipMap_CL4toCL8      (void*, TQAImage*, UInt32);
void BltMipMap_32ARGBtoAlpha8(void*, TQAImage*, UInt32);

// sst1Timer.c
void txtDisplayNum    (TQADrawContext *dp, UInt32, UInt32, UInt32, UInt32);
void txtDisplayLabel  (TQADrawContext *dp, UInt32, UInt32);
void txtDisplayTexLoad(TQADrawContext *dp, UInt32, UInt32, UInt32);

// tnsl3Dfx.c
UInt16* utilPrepare_RGB16 (UInt16*, UInt16*, UInt32, UInt32, UInt32);
UInt16* utilPrepare_ARGB16(UInt16*, UInt16*, UInt32, UInt32, UInt32);

void utilCopy_RGB32toRGB16  (UInt16*, UInt32*, UInt32, UInt32, UInt32);
void utilCopy_ARGB32toARGB16(UInt16*, UInt32*, UInt32, UInt32, UInt32);
void utilCopy_RGB16toRGB32  (UInt32*, UInt16*, UInt32, UInt32, UInt32);
void utilCopy_ARGB16toARGB32(UInt32*, UInt16*, UInt32, UInt32, UInt32);

void TesselateTriangle(	const TQADrawContext*, TQAVTexture*, TQAVTexture*, TQAVTexture*);

// tnslWindow.c
#if TNSL_WINDOW

typedef enum {
	kSearchStateForeground,
	kSearchStateBegin 			= kSearchStateForeground,
	kSearchStateBackground,
	kSearchStateNormal
} SearchProcState;

/* Where are we in the search proc?
 * The caller must reset this to kSearchStateBegin 
 * before every call to CopyBits or the color conversions
 * from our board's native format to the mac format will
 * not be correct.
 */
extern SearchProcState gSearchState; /* tnslWindow.c */

extern void 
SetupRaveCtxt(TQADrawPrivate* dp);

extern void
CleanupRaveCtxt(TQADrawPrivate* dp);

#endif /* TNSL_WINDOW */
TQAError 	CalcTextureSize( const TQAImage **pImage, UInt32 flags, GrLOD_t *outWidthLog2, GrLOD_t *outHeightLog2 );
TQAError 	CalcBitmapSize( const TQAImage *pImage, GrLOD_t *outWidthLog2, GrLOD_t *outHeightLog2 );
void 		TextureForceDelete(TQATexture* texture);

#ifdef __cplusplus
};
#endif


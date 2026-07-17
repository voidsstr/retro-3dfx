#ifndef __gl_pixel_h_
#define __gl_pixel_h_

/*
** Copyright 1991-1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/
#include "types.h"
#include "vertex.h"
#include "consts.h"

/* Not much for elegance, but it works. */
#define __GL_N_PIXEL_MAPS       (GL_PIXEL_MAP_A_TO_A - GL_PIXEL_MAP_I_TO_I + 1)
#define __GL_REMAP_PM(x)        ((x) - GL_PIXEL_MAP_I_TO_I)
#define __GL_PIXEL_MAP_I_TO_I   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_I)
#define __GL_PIXEL_MAP_S_TO_S   __GL_REMAP_PM(GL_PIXEL_MAP_S_TO_S)
#define __GL_PIXEL_MAP_I_TO_R   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_R)
#define __GL_PIXEL_MAP_I_TO_G   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_G)
#define __GL_PIXEL_MAP_I_TO_B   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_B)
#define __GL_PIXEL_MAP_I_TO_A   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_A)
#define __GL_PIXEL_MAP_R_TO_R   __GL_REMAP_PM(GL_PIXEL_MAP_R_TO_R)
#define __GL_PIXEL_MAP_G_TO_G   __GL_REMAP_PM(GL_PIXEL_MAP_G_TO_G)
#define __GL_PIXEL_MAP_B_TO_B   __GL_REMAP_PM(GL_PIXEL_MAP_B_TO_B)
#define __GL_PIXEL_MAP_A_TO_A   __GL_REMAP_PM(GL_PIXEL_MAP_A_TO_A)

/*
** Pixel format not explicitly defined by the spec, but implictly suggested.
** (for processing luminance alpha texture images)
*/
#define __GL_RED_ALPHA		1

/*
** Packed pixel types used for storing texture images
*/
enum {
    __GL_UNSIGNED_SHORT_5_6_5,
    __GL_UNSIGNED_SHORT_X_5_5_5,
    __GL_UNSIGNED_SHORT_1_5_5_5,
    __GL_UNSIGNED_SHORT_4_4_4_4_ARGB,
};

typedef struct __GLpixelMapHeadRec {
    GLint size;
    GLint tableId;
    union {
        GLint *mapI;		/* access index (integral) entries */
        __GLfloat *mapF;	/* access component (float) entries */
    } base;
} __GLpixelMapHead;

typedef struct __GLpixelTransferModeRec {
    __GLfloat r_scale, g_scale, b_scale, a_scale, d_scale;
    __GLfloat r_bias, g_bias, b_bias, a_bias, d_bias;
    __GLfloat zoomX;
    __GLfloat zoomY;

    GLint indexShift;
    GLint indexOffset;

    GLboolean mapColor;
    GLboolean mapStencil;
} __GLpixelTransferMode;

typedef struct __GLpixelPackModeRec {
    GLboolean swapEndian;
    GLboolean lsbFirst;

    GLuint lineLength;
    GLuint skipLines;
    GLuint skipPixels;
    GLuint alignment;
} __GLpixelPackMode;

typedef struct __GLpixelUnpackModeRec {
    GLboolean swapEndian;
    GLboolean lsbFirst;

    GLuint lineLength;
    GLuint skipLines;
    GLuint skipPixels;
    GLuint alignment;
} __GLpixelUnpackMode;

typedef struct __GLpixelStateRec {
    __GLpixelTransferMode transferMode;
    __GLpixelMapHead pixelMap[__GL_N_PIXEL_MAPS];
    __GLpixelPackMode packModes;
    __GLpixelUnpackMode unpackModes;
    GLuint pixelMapTableId;

    /*
    ** Read buffer.  Where pixel reads come from.
    */
    GLenum readBuffer;

    /*
    ** Read buffer specified by user.  May be different from readBuffer
    ** above.  If the user specifies GL_FRONT_LEFT, for example, then 
    ** readBuffer is set to GL_FRONT, and readBufferReturn to 
    ** GL_FRONT_LEFT.
    */
    GLenum readBufferReturn;
} __GLpixelState;

#define __GL_MT_SIG_IDENTITY 	0x00008421

typedef struct __GLpixelMachineRec {
    GLboolean modifyRGBA;	/* Is the RGBA path being modified? */
    GLboolean modifyCI;
    GLboolean modifyDepth;
    GLboolean modifyStencil;
    GLboolean fastRGBA;
    /* scaled values indicating what a red of 0 maps to, an alpha of 1 ... */
    GLfloat red0Mod, green0Mod, blue0Mod, alpha1Mod;
    GLfloat *redMap;		/* Lookup tables with no modification */
    GLfloat *greenMap;
    GLfloat *blueMap;
    GLfloat *alphaMap;
    GLfloat *iMap;
    GLvoid *iCurMap, *redCurMap, *greenCurMap, *blueCurMap, *alphaCurMap;
    GLboolean rgbaCurrent;	
    GLfloat *redModMap;		/* Lookup tables for modification path */
    GLfloat *greenModMap;
    GLfloat *blueModMap;
    GLfloat *alphaModMap;
    GLboolean iToICurrent;	/* Lookup table for modification of CI */
    GLfloat *iToIMap;
    GLboolean iToRGBACurrent;	/* Lookup tables from CI to RGBA */
    GLfloat *iToRMap;
    GLfloat *iToGMap;
    GLfloat *iToBMap;
    GLfloat *iToAMap;
} __GLpixelMachine;

extern void __glInitDefaultPixelMap(__GLcontext *gc, GLenum map);

/************************************************************************/

#define __GL_MAX_SPAN_SIZE	(__GL_MAX_MAX_VIEWPORT * 4 * sizeof(GLfloat))

struct __GLpixelSpanInfoRec {
    GLenum srcFormat, srcType;	/* Form of source image */
    const GLvoid *srcImage;	/* The source image */
    GLvoid *srcCurrent;		/* The current pointer into the source data */
    GLint srcRowIncrement;	/* Add this much to get to the next row */
    GLint srcGroupIncrement;	/* Add this much to get to the next group */
    GLint srcComponents;	/* (4 for RGBA, 1 for ALPHA, etc.) */
    GLint srcElementSize;	/* Size of one element (1 for BYTE) */
    GLint srcSwapBytes;		
    GLint srcLsbFirst;
    GLint srcSkipPixels, srcSkipLines;
    GLint srcLineLength;
    GLint srcAlignment;
    GLboolean srcPackedData;	/* True if source data is packed */
    GLint srcStartBit;		/* After applying skipPixels */

    GLenum dstFormat, dstType;	/* Form of destination image */
    const GLvoid *dstImage;	/* The destination image */
    GLvoid *dstCurrent;		/* The current pointer into the dest data */
    GLint dstRowIncrement;	/* Add this much to get to the next row */
    GLint dstGroupIncrement;	/* Add this much to get to the next group */
    GLint dstComponents;	/* (4 for RGBA, 1 for ALPHA, etc.) */
    GLint dstElementSize;	/* Size of one element (1 for BYTE) */
    GLint dstSwapBytes;		
    GLint dstLsbFirst;
    GLint dstSkipPixels, dstSkipLines;
    GLint dstLineLength;
    GLint dstAlignment;
    GLboolean dstPackedData;	/* True if destination data is packed */
    GLint dstStartBit;		/* After applying skipPixels */

    __GLfloat zoomx, zoomy;
    GLint width, height, depth;	/* Size of image */
    __GLfloat readX, readY;	/* Reading coords (CopyPixels, ReadPixels) */
    __GLfloat x, y;		/* Effective raster coordinates */
    GLint startCol, startRow;	/* First actual pixel goes here */
    GLint endCol;		/* Last column rendered (minus coladd) */
    GLint columns, rows;	/* Taking zoomx, zoomy into account */
    GLboolean overlap;		/* Do CopyPixels src/dest regions overlap? */
    GLint rowsUp, rowsDown;	/* Stuff for overlapping CopyPixels regions */
    GLint rowadd, coladd;	/* Adders for incrementing the col or row */
    __GLfloat rendZoomx;	/* effective zoomx for render span */
    __GLzValue fragz;		/* save this computation in the span walker */
    __GLfloat rpyUp, rpyDown;
    GLint startUp, startDown;
    GLint readUp, readDown;
    GLvoid *redMap, *greenMap, *blueMap, *alphaMap;
    GLvoid *indexMap;
    GLshort *pixelArray;	/* Array of pixel replication counts (if    */
				/* zoomx < -1 or zoomx > 1) or pixels to   */
				/* skip (if zoomx < 1 and zoomx > -1).     */
    GLfloat rscale, gscale, bscale, ascale; /* generic scale values */
    GLfloat rbias, gbias, bbias, abias;     /* generic bias values */
    GLboolean fbScale;		/* pixels are scaled by fb scale factors */

    /*
    ** A pile of span routines used by the DrawPixels, ReadPixels, and 
    ** CopyPixels functions.
    */
    GLint numSpanMods; 		/* number of span modifiers current in use */
    void (*spanReader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *outspan);
    void (*(spanModifier[13]))(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		              GLvoid *inspan, GLvoid *outspan);
    void (*spanRender)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *inspan);

    /*
    ** These fields are used only for selecting spanModifiers.  The
    ** span modifier routines themselves do not use them.  They are
    ** in this record because they need to be passed through copyImage.
    */
    GLboolean zeroFillAlpha;        /* fill alpha with zero value */
    GLboolean applyClamp;           /* to clamp or not to clamp */
    GLboolean applyFbScale;         /* apply frame buffer scale/unscale */
    GLboolean nonColorComp;         /* not color components (eg hgram) */
    GLint dim;                      /* dimensionality of calling routine */
};

/* px_api.c */
extern GLboolean __glCheckDrawPixelArgs(__GLcontext *gc, GLsizei width, 
					GLsizei height, GLenum format, 
					GLenum type);
void __glPixelSetColorScales(__GLcontext *);

/* px_paths.c */
GLboolean __glClipDrawPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glComputeSpanPixelArray(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glLoadUnpackModes(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			 GLboolean packed);
void __glInitDrawPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLint width, GLint height, GLenum format, 
                            GLenum type, const GLvoid *pixels);

void __glDrawPixelSpans(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glDrawPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glDrawPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glDrawPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);

void __glOptPickDrawPixels(__GLcontext *gc, GLint width, GLint height,
		            GLenum format, GLenum type, const GLvoid *pixels,
			    GLboolean packed);

void __glSlowPickDrawPixels(__GLcontext *gc, GLint width, GLint height,
                            GLenum format, GLenum type, const GLvoid *pixels,
			    GLboolean packed);
void __glGenericPickDrawPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
GLboolean __glClipReadPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glLoadPackModes(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glInitReadPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLint x, GLint y, GLint width, GLint height, 
                            GLenum format, GLenum type, const GLvoid *pixels);

void __glReadPixelSpans(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glReadPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glReadPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glReadPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glSlowPickReadPixels(__GLcontext *gc, GLint x, GLint y,
                            GLsizei width, GLsizei height,
                            GLenum format, GLenum type, const GLvoid *pixels);
void __glGenericPickReadPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
GLboolean __glClipCopyPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glInitCopyPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                            GLint x, GLint y, GLint width, GLint height, 
                            GLenum format);
void __glCopyPixelSpans(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glCopyPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glCopyPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glCopyPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glCopyPixelsOverlapping(__GLcontext *gc, 
                               __GLpixelSpanInfo *spanInfo, GLint modifiers);
void __glSlowPickCopyPixels(__GLcontext *gc, GLint x, GLint y, GLint width,
                            GLint height, GLenum type);
void __glGenericPickCopyPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glCopyImage1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glCopyImage2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glCopyImageSpans(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);

void __glGenericPickStoreTexture(__GLcontext *gc, struct __GLtextureRec *tex,
                                 __GLpixelSpanInfo *spanInfo,
                                 GLboolean applyPixelTransfer);

void __glGenericPickGetTexture(__GLcontext *gc, struct __GLtextureRec *tex,
                               __GLpixelSpanInfo *spanInfo,
                               GLboolean applyPixelTransfer);

void __glGenericPickCopyImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                              GLboolean applyPixelTransfer);
void __glInitReadImageSrcInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			      GLint x, GLint y,
			      GLsizei width, GLsizei height);
void __glGenericPickReadImage(__GLcontext *gc, struct __GLtextureRec *tex, 
                              __GLpixelSpanInfo *spanInfo);
GLint __glGenericPickCopyImageSpan(__GLcontext *gc, 
		    __GLpixelSpanInfo *spanInfo, GLboolean applyPixelTransfer);
void __glInitImagePack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLint width, GLint height, GLenum format, GLenum type,
                       const GLvoid *buf);
void __glInitMemLoad(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			GLenum internalFormat, const GLvoid *buf);
void __glInitMemGet(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			GLsizei width, GLsizei height, GLenum internalFormat,
			const GLvoid *buf);
void __glInitMemUnpack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			GLsizei width, GLsizei height, GLsizei depth,
			GLenum format, GLenum type, const GLvoid *buf,
			GLboolean dltex);
void __glInitMemPack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			GLsizei width, GLsizei height, GLsizei depth,
			GLenum format, GLenum type, const GLvoid *buf);
void __glLateScissorRender(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			   GLvoid *inspan);


/* px_modify.c */
void __glBuildRGBAModifyTables(__GLcontext *gc, __GLpixelMachine *pm);
void __glBuildItoIModifyTables(__GLcontext *gc, __GLpixelMachine *pm);
void __glBuildItoRGBAModifyTables(__GLcontext *gc, 
				  __GLpixelMachine *pm);
void __glSpanModifyRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		        GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyABGR(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		        GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyBGRA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		        GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyRed(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		       GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyBlue(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		        GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyGreen(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			 GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			 GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyRGB(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyBGR(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyLuminance(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyLuminanceAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                  GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyRedAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyDepth(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyStencil(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                           GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyCI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                      GLvoid *inspan, GLvoid *outspan);

/* px_pack.c */
void __glInitPacker(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glSpanReduceRed(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceGreen(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceBlue(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceRGB(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceBGR(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceLuminance(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceLuminanceAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                  GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceRedAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *inspan, GLvoid *outspan);
void __glSpanReorderABGR(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *inspan, GLvoid *outspan);
void __glSpanReorderBGRA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceRedNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceGreenNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceBlueNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceAlphaNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceRGBNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceBGRNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceRedAlphaNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *inspan, GLvoid *outspan);
void __glSpanReorderABGRNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *inspan, GLvoid *outspan);
void __glSpanReorderBGRANS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *inspan, GLvoid *outspan);

void __glSpanPackUbyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanPackByte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPackUshort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanPackShort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanPackUint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPack332Ubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPack4444Ushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPackARGB4444Ushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPack5551Ushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPack8888Uint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPack_10_10_10_2_Uint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPackX555Ushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPack1555Ushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPack565Ushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPackInt(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                     GLvoid *inspan, GLvoid *outspan);
void __glSpanPackUbyteI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanPackByteI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanPackUshortI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanPackShortI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanPackUintI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanPackIntI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPackNonCompByte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			GLvoid *inspan, GLvoid *outspan);
void __glSpanPackNonCompUbyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			GLvoid *inspan, GLvoid *outspan);
void __glSpanPackNonCompShort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			GLvoid *inspan, GLvoid *outspan);
void __glSpanPackNonCompUshort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			GLvoid *inspan, GLvoid *outspan);
void __glSpanPackNonCompInt(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			GLvoid *inspan, GLvoid *outspan);
void __glSpanPackNonCompUint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			GLvoid *inspan, GLvoid *outspan);

void __glSpanCopy(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
void __glSpanPackBitmap(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	                GLvoid *inspan, GLvoid *outspan);

/* px_read.c */
void __glSpanReadRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *span);
void __glSpanReadRGBA2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *span);
void __glSpanReadCI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                    GLvoid *span);
void __glSpanReadCI2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                     GLvoid *span);
void __glSpanReadDepth(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *span);
void __glSpanReadDepth2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *span);
void __glSpanReadDepthUint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			   GLvoid *span);
void __glSpanReadDepthUint2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			    GLvoid *span);
void __glSpanReadStencil(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                         GLvoid *span);
void __glSpanReadStencil2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                          GLvoid *span);

/* px_render.c */
void __glSlowDrawPixelsStore(__GLcolorBuffer *cfb, const __GLfragment *frag);
void __glSpanRenderRGBubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *span);
void __glSpanRenderRGBubyte2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *span);
void __glSpanRenderRGBAubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *span);
void __glSpanRenderRGBAubyte2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                              GLvoid *span);
void __glSpanRenderDepthUint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			     GLvoid *span);
void __glSpanRenderDepthUint2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			      GLvoid *span);
void __glSpanRenderStencilUshort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                 GLvoid *span);
void __glSpanRenderStencilUshort2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                  GLvoid *span);
void __glSpanRenderStencilUbyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                GLvoid *span);
void __glSpanRenderStencilUbyte2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                 GLvoid *span);
void __glSpanRenderCIuint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			  GLvoid *span);
void __glSpanRenderCIuint2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			   GLvoid *span);
void __glSpanRenderCIushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *span);
void __glSpanRenderCIushort2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *span);
void __glSpanRenderCIubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                           GLvoid *span);
void __glSpanRenderCIubyte2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *span);
void __glSpanRenderCIubyte3(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *span);
void __glSpanRenderCIubyte4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *span);
void __glSpanRenderRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        GLvoid *span);
void __glSpanRenderRGBA2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *span);
void __glSpanRenderDepth(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *span);
void __glSpanRenderDepth2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                          GLvoid *span);
void __glSpanRenderCI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                      GLvoid *span);
void __glSpanRenderCI2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *span);
void __glSpanRenderStencil(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                           GLvoid *span);
void __glSpanRenderStencil2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *span);

/* px_unpack.c */
GLint __glElementsPerGroup(GLenum format, GLenum type);
GLint __glBytesPerElement(GLenum type);
void __glInitUnpacker(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glSpanUnpackBitmap(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                          GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackBitmap2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                           GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackRGBubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                            GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackIndexUbyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                              GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackRGBAubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanSwapBytes2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanSwapBytes2Dst(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                           GLvoid *inspan, GLvoid *outspan);
void __glSpanSwapAndSkipBytes2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                               GLvoid *inspan, GLvoid *outspan);
void __glSpanSwapBytes4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanSwapBytes4Dst(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                           GLvoid *inspan, GLvoid *outspan);
void __glSpanSwapAndSkipBytes4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                               GLvoid *inspan, GLvoid *outspan);
void __glSpanSkipPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanSkipPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanSkipPixels4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanSlowSkipPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanSlowSkipPixels4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanAlignPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                          GLvoid *inspan, GLvoid *outspan);
void __glSpanAlignPixels2Dst(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanAlignPixels4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                          GLvoid *inspan, GLvoid *outspan);
void __glSpanAlignPixels4Dst(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackUbyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackByte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackUshort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                          GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackShort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackUint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackInt(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpack332Ubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpack4444Ushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackARGB4444Ushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpack5551Ushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpack8888Uint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpack_10_10_10_2_Uint(__GLcontext *gc, 
		       __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackX555Ushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpack1555Ushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpack565Ushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackUbyteI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                          GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackByteI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackUshortI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                           GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackShortI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                          GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackUintI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackIntI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackNonCompUint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			GLvoid *inspan, GLvoid *outspan);
void __glSpanClampNegPreFloat(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			      GLvoid *inspan, GLvoid *outspan);
void __glSpanClampPreFloat(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			   GLvoid *inspan, GLvoid *outspan);
void __glSpanClampPostFloat(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			    GLvoid *inspan, GLvoid *outspan);
void __glSpanClampRed(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		      GLvoid *inspan, GLvoid *outspan);
void __glSpanClampGreen(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanClampBlue(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		       GLvoid *inspan, GLvoid *outspan);
void __glSpanClampAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanClampRGB(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanClampRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanClampSigned(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandRed(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandGreen(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandBlue(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandRGB(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandBGR(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandLuminance(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandLuminanceAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                  GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandRedAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandRedNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandGreenNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandBlueNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandAlphaNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandRGBNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandBGRNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandLuminanceNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandLuminanceAlphaNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                  GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandRedAlphaNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandIntensityNS(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *inspan, GLvoid *outspan);

void __glSpanScaleRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanScaleABGR(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanScaleBGRA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanPreReorderABGR(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanPreReorderBGRA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanPreUnscaleRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			    GLvoid *inspan, GLvoid *outspan);
void __glSpanPostUnscaleRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			     GLvoid *inspan, GLvoid *outspan);
void __glSpanUnscaleABGR(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			 GLvoid *inspan, GLvoid *outspan);
void __glSpanUnscaleBGRA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			 GLvoid *inspan, GLvoid *outspan);
void __glSpanZeroFillAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandIntensity(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			GLvoid *inspan, GLvoid *outspan);

void __glSpanPackL8ToXRGB1555(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                                GLvoid *inspan, GLvoid *outspan);
void __glSpanPackRGB8ToXRGB1555(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                                GLvoid *inspan, GLvoid *outspan);
void __glSpanPackRGBA8ToXRGB1555(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                                 GLvoid *inspan, GLvoid *outspan);
void __glSpanPackL8ToRGB565(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                              GLvoid *inspan, GLvoid *outspan);
void __glSpanPackRGB8ToRGB565(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                              GLvoid *inspan, GLvoid *outspan);
void __glSpanPackRGBA8ToRGB565(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                          GLvoid *inspan, GLvoid *outspan);
void __glSpanPackLA8ToARGB1555(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                                GLvoid *inspan, GLvoid *outspan);
void __glSpanPackRGBA8ToARGB1555(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                                 GLvoid *inspan, GLvoid *outspan);
void __glSpanPackLA8ToARGB4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                                GLvoid *inspan, GLvoid *outspan);
void __glSpanPackRGBA8ToARGB4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                                 GLvoid *inspan, GLvoid *outspan);

#endif /* __gl_pixel_h_ */

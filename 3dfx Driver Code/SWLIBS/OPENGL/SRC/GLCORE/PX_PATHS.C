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

#include <math.h>
#include "context.h"
#include "global.h"
#include "pixel.h"
#include "imports.h"

/*
** Notes on pixel path cleanup work TBD
**
** So far the main goal of the pixel path cleanup has been to have only
** one instance of the pixel transfer code.  Pixel transfer includes
** arithmetic, LUT, etc.
** This has been accomplished for the pixel tranfer code beginning with the
** convolution step for all of the pixel paths supported in this file.  
** Other cleanup items are listed below.
** 
** Merge color table into this file
** 
** Color tables are loaded using the copyImage and readImage paths
** through the Expand to RGBA step, then they do some scale/bias and
** conversion operations.  The gets for all three paths run through the pixel 
** packing path.  These paths should be merged into PickSpanModifiers.
** Part of this process may involve changing the scale/bias operators to
** get the scale and bias from spanInfo instead of the context, which
** would allow the scale and bias set for color table to be used.
** 
** Create a flag word that controls the pixel path operation.
** 
** Once each step in the pixel pipeline is controlled by a flag in the
** flag word, this flag word can be used to easily identify states which
** can be optimized using hardware or software features.  Also, unchanged
** states can be identified, which can allow skipping reexecution of the pick
** code.  Part of the process of creating the flag word should include
** identification of the corresponding specification steps for each
** step in the pixel path implementation.  The code in PickSpanModifiers
** that sets up the host of flags that control the pixel paths has been
** left a bit ugly in anticipation of the implementation of the flag word,
** which should make the code obsolete.
** 
*/

/*
** This structure is used only for communication with the PickSpanModifiers
** routine.  The first set of fields contain information that is passed
** into the routine; the second set of fields contain information that
** is passed back to the calling routine.
*/
#define __GL_SRCDST_FB  	1
#define __GL_SRCDST_MEM 	2

#define __GL_PIXPATH_DRAWPIX	1
#define __GL_PIXPATH_READPIX	2
#define __GL_PIXPATH_COPYPIX	3
#define __GL_PIXPATH_READIMAGE	4
#define __GL_PIXPATH_COPYIMAGE	5

typedef struct __GLpixelSpanModInfo {
					/* in values begin here */
	GLint srcType;			/* FB, MEM, etc. */
	GLint dstType;			/* FB, MEM, etc. */
	GLint pixelPath;		/* type of pixel path */
	GLboolean applyPixelTransfer; 	/* apply pix xfer to pix load */
	GLboolean zeroFillAlpha;	/* fill alpha with zero value */
	GLboolean applyClamp;		/* to clamp or not to clamp */
	void (*reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *outspan);
	void (*render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *inspan);
} __GLpixelSpanModInfo;


/*
** Determines if the given type is a packed_pixel format
*/
static GLboolean isPackedType(GLenum type)
{
    switch (type) {
    case GL_UNSIGNED_BYTE_3_3_2_EXT:
    case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
    case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
    case GL_UNSIGNED_INT_8_8_8_8_EXT:
    case GL_UNSIGNED_INT_10_10_10_2_EXT:
    case __GL_UNSIGNED_SHORT_5_6_5:
    case __GL_UNSIGNED_SHORT_X_5_5_5:
    case __GL_UNSIGNED_SHORT_1_5_5_5:
    case __GL_UNSIGNED_SHORT_4_4_4_4_ARGB:
        return GL_TRUE;
    default:
        return GL_FALSE;
    }
}

/*
** Picks span modifier routines for DrawPixel, ReadPixel, CopyPixel,
** CopyImage (TexImage, GetTexImage), and CopyTexture.
*/
static
void PickSpanModifiers(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			    __GLpixelSpanModInfo *spanModInfo)
{
    GLint spanCount = spanInfo->numSpanMods;
    __GLpixelMachine *pm;
    void (*reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    void (*render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
    GLboolean isIndex;
    GLboolean modify;
    GLboolean skip;

    GLenum srcType, dstType, dstFormat, srcFormat;
    GLboolean srcSwap, dstSwap;
    GLboolean srcAlign, dstAlign;
    GLboolean srcConvert, dstConvert;
    GLboolean srcExpand, dstReduce;
    GLboolean srcClamp, dstClamp;

    GLboolean zoomx1;           /* -1 <= zoomx <= 1? */
    GLboolean zoomx2;           /* zoomx <= -1 || zoomx >= 1 */
    __GLfloat zoomx;
    GLboolean packedUserData;

    /*
    ** NOTE:
    ** The code that sets up the flags here is more than a bit unoptimal
    ** and ugly, but it has been left that way in anticipation of the
    ** implementation of a pixel path flag word, which should render this
    ** code mostly obsolete.  See the comment on work to be done above.
    */

    dstType = spanInfo->dstType;
    dstFormat = spanInfo->dstFormat;
    srcType = spanInfo->srcType;
    srcFormat = spanInfo->srcFormat;

    pm = &gc->pixel;

    if (spanModInfo->dstType == __GL_SRCDST_FB) {
	/* Only apply PixelZoom if the FB is the destination. */
	zoomx = gc->state.pixel.transferMode.zoomX;
    } else {
	zoomx = 1.0;
    }
    if (zoomx >= (__GLfloat) -1.0 && zoomx <= __glOne) {
	zoomx1 = GL_TRUE;
    } else {
	zoomx1 = GL_FALSE;
    }
    if (zoomx <= (__GLfloat) -1.0 || zoomx >= __glOne) {
	zoomx2 = GL_TRUE;
    } else {
	zoomx2 = GL_FALSE;
    }
    packedUserData = spanInfo->srcPackedData && zoomx2;
    if (zoomx2 || srcType == GL_BITMAP) {
	skip = GL_FALSE;
    } else {
	skip = GL_TRUE;
    }

    if (spanInfo->srcSwapBytes && spanInfo->srcElementSize > 1) {
	srcSwap = GL_TRUE;
    } else {
	srcSwap = GL_FALSE;
    }
    if (spanInfo->dstSwapBytes && spanInfo->dstElementSize > 1) {
	dstSwap = GL_TRUE;
    } else {
	dstSwap = GL_FALSE;
    }

    switch(srcFormat) {
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_RGB:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case __GL_RED_ALPHA:
      case GL_RGBA:
      case GL_ABGR_EXT:
      case GL_BGR_EXT:
      case GL_BGRA_EXT:
	modify = spanModInfo->applyPixelTransfer && pm->modifyRGBA;
	break;
      case GL_DEPTH_COMPONENT:
	modify = spanModInfo->applyPixelTransfer && pm->modifyDepth;
	break;
      case GL_STENCIL_INDEX:
	modify = spanModInfo->applyPixelTransfer && pm->modifyStencil;
	break;
      case GL_COLOR_INDEX:
	modify = spanModInfo->applyPixelTransfer && pm->modifyCI;
	break;
    }

    if (spanModInfo->pixelPath == __GL_PIXPATH_COPYIMAGE) {

	if ( !modify &&
	    ((srcFormat == dstFormat) || 
	     (srcFormat == GL_LUMINANCE_ALPHA && dstFormat == __GL_RED_ALPHA) ||
	     (srcFormat == __GL_RED_ALPHA && dstFormat == GL_LUMINANCE_ALPHA) ||
	     (srcFormat == GL_LUMINANCE && dstFormat == GL_RED) ||
	     (srcFormat == GL_RED && dstFormat == GL_LUMINANCE))) {
	    srcExpand = GL_FALSE;
	    dstReduce = GL_FALSE;
	} else {
	    srcExpand = GL_TRUE;
	    dstReduce = GL_TRUE;
	}
    } else {
	srcExpand = GL_TRUE;
	dstReduce = GL_TRUE;
    }

    if (srcType != GL_BITMAP &&
	(((unsigned long) (spanInfo->srcImage)) & /* XXXportability */
	    (spanInfo->srcElementSize - 1))) {
	srcAlign = GL_TRUE;
    } else {
	srcAlign = GL_FALSE;
    }
    if (dstType != GL_BITMAP &&
	(((unsigned long) (spanInfo->dstImage)) & /* XXXportability */
	    (spanInfo->dstElementSize - 1))) {
	dstAlign = GL_TRUE;
    } else {
	dstAlign = GL_FALSE;
    }
    /* In CopyImage bitmaps get converted; otherwise they don't. */
    if (srcType == GL_FLOAT ||  spanInfo->nonColorComp ||
       ((spanModInfo->pixelPath != __GL_PIXPATH_COPYIMAGE) && 
					(dstType == GL_BITMAP))) {
	srcConvert = GL_FALSE;
    } else {
	srcConvert = GL_TRUE;
    }
    if (dstType == GL_FLOAT || spanInfo->nonColorComp ||
       ((spanModInfo->pixelPath != __GL_PIXPATH_COPYIMAGE) && 
					(dstType == GL_BITMAP))) {
	dstConvert = GL_FALSE;
    } else {
	dstConvert = GL_TRUE;
    }
    /*
    ** Clamp types only if index or not modifying (because converting
    ** float types means clamping, and that is only done if not modifying),
    ** and only if they might need clamping (UNSIGNED types never do).
    */
    if ( (!spanInfo->applyClamp) || 
	    (spanModInfo->srcType == __GL_SRCDST_FB) ||
	    srcType == GL_BITMAP || srcType == GL_UNSIGNED_BYTE || 
	    srcType == GL_UNSIGNED_SHORT || srcType == GL_UNSIGNED_INT ||
	    srcFormat == GL_COLOR_INDEX || srcFormat == GL_STENCIL_INDEX ||
	    (srcFormat == GL_DEPTH_COMPONENT && pm->modifyDepth) ||
	    (srcFormat != GL_DEPTH_COMPONENT && pm->modifyRGBA)) {
	srcClamp = GL_FALSE;
    } else {
	srcClamp = GL_TRUE;
    }
    if (spanModInfo->applyPixelTransfer && srcClamp) {
	dstClamp = GL_TRUE;
    } else {
	dstClamp = GL_FALSE;
    }
	    
    /* At this point we can pick fastpaths to optimize trivial data copies,
     * including conversion to packed internal formats.
     */
    if (spanModInfo->pixelPath == __GL_PIXPATH_COPYIMAGE &&
        srcType == GL_UNSIGNED_BYTE &&
        (dstType == GL_UNSIGNED_BYTE || isPackedType(dstType)) &&
        !skip && !modify && !srcAlign && !dstAlign && !srcClamp && !dstClamp) {

        /* Look for trivial memcpy case */
        if (srcType == dstType && !srcExpand && !dstReduce) {
            goto done;
        }

        /* Optimized fastpaths for pack_pixel format downloads.
         * Unsupported types fall through to the slow paths.
         */
        switch (dstType) {
        case __GL_UNSIGNED_SHORT_5_6_5:
            switch (spanInfo->srcComponents) {
            case 1:
                spanInfo->spanModifier[spanCount++] =
                    __glSpanPackL8ToRGB565;
                goto done;
            case 3:
                spanInfo->spanModifier[spanCount++] =
                    __glSpanPackRGB8ToRGB565;
                goto done;
            case 4:
                spanInfo->spanModifier[spanCount++] =
                    __glSpanPackRGBA8ToRGB565;
                goto done;
            }
            break;
        case __GL_UNSIGNED_SHORT_X_5_5_5:
            switch (spanInfo->srcComponents) {
            case 1:
                spanInfo->spanModifier[spanCount++] =
                    __glSpanPackL8ToXRGB1555;
                goto done;
            case 3:
                spanInfo->spanModifier[spanCount++] =
                    __glSpanPackRGB8ToXRGB1555;
                goto done;
            case 4:
                spanInfo->spanModifier[spanCount++] =
                    __glSpanPackRGBA8ToXRGB1555;
                goto done;
            }
            break;
        case __GL_UNSIGNED_SHORT_1_5_5_5:
            switch (spanInfo->srcComponents) {
            case 2:
                spanInfo->spanModifier[spanCount++] =
                    __glSpanPackLA8ToARGB1555;
                goto done;
            case 4:
                spanInfo->spanModifier[spanCount++] =
                    __glSpanPackRGBA8ToARGB1555;
                goto done;
            }
            break;
        case __GL_UNSIGNED_SHORT_4_4_4_4_ARGB:
            switch (spanInfo->srcComponents) {
            case 2:
                spanInfo->spanModifier[spanCount++] =
                    __glSpanPackLA8ToARGB4;
                goto done;
            case 4:
                spanInfo->spanModifier[spanCount++] =
                    __glSpanPackRGBA8ToARGB4;
                goto done;
            }
            break;
        }
    }

    switch(dstFormat) {
      case GL_RGB:
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_ALPHA:
      case GL_RGBA:
      case GL_ABGR_EXT:
      case GL_BGR_EXT:
      case GL_BGRA_EXT:
      case __GL_RED_ALPHA:
      case GL_DEPTH_COMPONENT:
	isIndex = GL_FALSE;
	break;
      case GL_STENCIL_INDEX:
      case GL_COLOR_INDEX:
	isIndex = GL_TRUE;
	break;
      default:
	assert(0);
    }

    switch(spanModInfo->srcType) {
      case __GL_SRCDST_MEM:

	/* 
	**  convert data into a packed readable format 
	** (RED, BYTE), (LUMINANCE, UNSIGNED_INT), etc...  This stage
	** simply packs the user's data, but performs no conversion on it.
	**
	** Packing can consist of:
	**  - aligning the data
	**  - skipping pixels if |xzoom| is < 1
	**  - swapping bytes if necessary
	*/
	if (srcSwap) {
	    if (skip) {
		if (spanInfo->srcElementSize == 2) {
		    spanInfo->spanModifier[spanCount++] = 
			    __glSpanSwapAndSkipBytes2;
		} else /* spanInfo->srcElementSize == 4 */ {
		    spanInfo->spanModifier[spanCount++] = 
			    __glSpanSwapAndSkipBytes4;
		}
	    } else {
		if (spanInfo->srcElementSize == 2) {
		    spanInfo->spanModifier[spanCount++] = __glSpanSwapBytes2;
		} else /* spanInfo->srcElementSize == 4 */ {
		    spanInfo->spanModifier[spanCount++] = __glSpanSwapBytes4;
		}
	    }
	} else if (srcAlign) {
	    if (skip) {
		if (spanInfo->srcElementSize == 2) {
		    spanInfo->spanModifier[spanCount++] = __glSpanSlowSkipPixels2;
		} else /* spanInfo->srcElementSize == 4 */ {
		    spanInfo->spanModifier[spanCount++] = __glSpanSlowSkipPixels4;
		}
	    } else {
		if (spanInfo->srcElementSize == 2) {
		    spanInfo->spanModifier[spanCount++] = __glSpanAlignPixels2;
		} else /* spanInfo->srcElementSize == 4 */ {
		    spanInfo->spanModifier[spanCount++] = __glSpanAlignPixels4;
		}
	    }
	} else if (skip) {
	    if (spanInfo->srcElementSize == 1) {
		spanInfo->spanModifier[spanCount++] = __glSpanSkipPixels1;
	    } else if (spanInfo->srcElementSize == 2) {
		spanInfo->spanModifier[spanCount++] = __glSpanSkipPixels2;
	    } else /* spanInfo->srcElementSize == 4 */ {
		spanInfo->spanModifier[spanCount++] = __glSpanSkipPixels4;
	    }
	}

	/* 
	** Conversion to float
	** All formats are converted into floating point (including GL_BITMAP).
	*/
	if (srcConvert) {
	    if (srcFormat == GL_COLOR_INDEX || srcFormat == GL_STENCIL_INDEX) {
		/* Index conversion */
		switch(srcType) {
		  case GL_BYTE:
		    spanInfo->spanModifier[spanCount++] = __glSpanUnpackByteI;
		    break;
		  case GL_UNSIGNED_BYTE:
		    spanInfo->spanModifier[spanCount++] = __glSpanUnpackUbyteI;
		    break;
		  case GL_SHORT:
		    spanInfo->spanModifier[spanCount++] = __glSpanUnpackShortI;
		    break;
		  case GL_UNSIGNED_SHORT:
		    spanInfo->spanModifier[spanCount++] = __glSpanUnpackUshortI;
		    break;
		  case GL_INT:
		    spanInfo->spanModifier[spanCount++] = __glSpanUnpackIntI;
		    break;
		  case GL_UNSIGNED_INT:
		    spanInfo->spanModifier[spanCount++] = __glSpanUnpackUintI;
		    break;
		}
	    } else {
		/* Component conversion */
		switch(srcType) {
		  case GL_BYTE:
		    spanInfo->spanModifier[spanCount++] = __glSpanUnpackByte;
		    break;
		  case GL_UNSIGNED_BYTE:
		    spanInfo->spanModifier[spanCount++] = __glSpanUnpackUbyte;
		    break;
		  case GL_SHORT:
		    spanInfo->spanModifier[spanCount++] = __glSpanUnpackShort;
		    break;
		  case GL_UNSIGNED_SHORT:
		    spanInfo->spanModifier[spanCount++] = __glSpanUnpackUshort;
		    break;
		  case GL_INT:
		    spanInfo->spanModifier[spanCount++] = __glSpanUnpackInt;
		    break;
		  case GL_UNSIGNED_INT:
		    spanInfo->spanModifier[spanCount++] = __glSpanUnpackUint;
		    break;
		  case GL_UNSIGNED_BYTE_3_3_2_EXT:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanUnpack332Ubyte;
		    /*
		    ** So why the heck are we setting srcComponents here??
		    ** 
		    ** The packed pixel routines use the correct
		    ** number of components implicitly.  Other routines
		    ** that come after the packed pixel routines need
		    ** to know the number of components that were produced
		    ** when the packed pixels were unpacked.  Setting
		    ** srcComponents to its expanded value only
		    ** works because srcComponents is not used by the
		    ** packed pixel routines, which assume the number
		    ** of components is always one.
		    **
		    ** We can't set the value at an earlier point, because 
		    ** the number of components needs to be 1 for the 
		    ** computation of row sizes, etc.  
		    **
		    ** The same principle applies to the packing of 
		    ** packed pixels.  
		    */
		    spanInfo->srcComponents = 3;
		    break;
		  case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanUnpack4444Ushort;
		    spanInfo->srcComponents = 4;
		    break;
		  case __GL_UNSIGNED_SHORT_4_4_4_4_ARGB:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanUnpackARGB4444Ushort;
		    spanInfo->srcComponents = 4;
		    break;
		  case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanUnpack5551Ushort;
		    spanInfo->srcComponents = 4;
		    break;
		  case __GL_UNSIGNED_SHORT_X_5_5_5:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanUnpackX555Ushort;
		    spanInfo->srcComponents = 3;
		    break;
		  case __GL_UNSIGNED_SHORT_1_5_5_5:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanUnpack1555Ushort;
		    spanInfo->srcComponents = 4;
		    break;
		  case __GL_UNSIGNED_SHORT_5_6_5:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanUnpack565Ushort;
		    spanInfo->srcComponents = 3;
		    break;
		  case GL_UNSIGNED_INT_8_8_8_8_EXT:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanUnpack8888Uint;
		    spanInfo->srcComponents = 4;
		    break;
		  case GL_UNSIGNED_INT_10_10_10_2_EXT:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanUnpack_10_10_10_2_Uint;
		    spanInfo->srcComponents = 4;
		    break;
		}
	    }
	} else if (spanInfo->nonColorComp) {
	    /* only needed for hgram so far; only srcType is Uint */
	    spanInfo->spanModifier[spanCount++] = __glSpanUnpackNonCompUint;
	}


	/* Optimization: clamp early */
	if (srcClamp) {
	    /* if unsigned type, clamp 0 to 1 */
	    spanInfo->spanModifier[spanCount++] = __glSpanClampPreFloat;
	}

	if (srcType == GL_BITMAP) {
	    if (zoomx2) {
		spanInfo->spanModifier[spanCount++] = __glSpanUnpackBitmap2;
	    } else {
		spanInfo->spanModifier[spanCount++] = __glSpanUnpackBitmap;
	    }
	}

	/* 
	**  Expansion to RGBA, Modification and color scaling
	**
	** Spans are modified if necessary (color biasing, maps, shift,
	** scale), and RGBA colors are scaled.  Also, all RGBA derivative
	** formats (RED, LUMINANCE, ALPHA, etc.) are converted to RGBA.
	** The only four span formats that survive this stage are:
	**
	** (COLOR_INDEX, FLOAT),
	** (STENCIL_INDEX, FLOAT),
	** (DEPTH_COMPONENT, FLOAT),
	** (RGBA, FLOAT),
	*/

	if (srcExpand) {
	    switch(srcFormat) {
	      case GL_RED:
		if (pm->modifyRGBA) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyRed;
		} else {
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanExpandRed : __glSpanExpandRedNS;
		}
		if (spanInfo->zeroFillAlpha) {
		    spanInfo->spanModifier[spanCount++]=__glSpanZeroFillAlpha;
		}
		break;
	      case GL_GREEN:
		if (pm->modifyRGBA) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyGreen;
		} else {
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanExpandGreen : __glSpanExpandGreenNS;
		}
		if (spanInfo->zeroFillAlpha) {
		    spanInfo->spanModifier[spanCount++]=__glSpanZeroFillAlpha;
		}
		break;
	      case GL_BLUE:
		if (pm->modifyRGBA) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyBlue;
		} else {
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanExpandBlue : __glSpanExpandBlueNS;
		}
		if (spanInfo->zeroFillAlpha) {
		    spanInfo->spanModifier[spanCount++]=__glSpanZeroFillAlpha;
		}
		break;
	      case GL_ALPHA:
		if (pm->modifyRGBA) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyAlpha;
		} else {
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanExpandAlpha : __glSpanExpandAlphaNS;
		}
		break;
	      case GL_RGB:
		if (pm->modifyRGBA) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyRGB;
		} else {
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanExpandRGB : __glSpanExpandRGBNS;
		}
		if (spanInfo->zeroFillAlpha) {
		    spanInfo->spanModifier[spanCount++]=__glSpanZeroFillAlpha;
		}
		break;
	      case GL_BGR_EXT:
		if (pm->modifyRGBA) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyBGR;
		} else {
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanExpandBGR : __glSpanExpandBGRNS;
		}
		if (spanInfo->zeroFillAlpha) {
		    spanInfo->spanModifier[spanCount++]=__glSpanZeroFillAlpha;
		}
		break;
	      case GL_LUMINANCE:
		if (pm->modifyRGBA) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyLuminance;
		} else {
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanExpandLuminance : __glSpanExpandLuminanceNS;
		}
		if (spanInfo->zeroFillAlpha) {
		    spanInfo->spanModifier[spanCount++]=__glSpanZeroFillAlpha;
		}
		break;
	      case GL_LUMINANCE_ALPHA:
		if (pm->modifyRGBA) {
		    spanInfo->spanModifier[spanCount++] = 
					    __glSpanModifyLuminanceAlpha;
		} else {
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanExpandLuminanceAlpha : 
			__glSpanExpandLuminanceAlphaNS;
		}
		break;
	      case __GL_RED_ALPHA:
		if (modify) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyRedAlpha;
		} else {
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanExpandRedAlpha : __glSpanExpandRedAlphaNS;
		}
		break;
	      case GL_RGBA:
		if (pm->modifyRGBA) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyRGBA;
		} else {
		    if (spanInfo->applyFbScale) {
			spanInfo->spanModifier[spanCount++] = __glSpanScaleRGBA;
		    }
		}
		break;
	      case GL_DEPTH_COMPONENT:
		if (pm->modifyDepth) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyDepth;
		} 
		break;
	      case GL_STENCIL_INDEX:
		if (pm->modifyStencil) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyStencil;
		} 
		break;
	      case GL_COLOR_INDEX:
		if (pm->modifyCI) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyCI;
		} 
		break;
	      case GL_ABGR_EXT:
		if (pm->modifyRGBA) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyABGR;
		} else {
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanScaleABGR : __glSpanPreReorderABGR;
		}
		break;
	      case GL_BGRA_EXT:
		if (pm->modifyRGBA) {
		    spanInfo->spanModifier[spanCount++] = __glSpanModifyBGRA;
		} else {
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanScaleBGRA : __glSpanPreReorderBGRA;
		}
		break;
	      case GL_INTENSITY:
		/* 
		** Modify should never be on, because this is an internal
		** format, and modify only happens when loading from user
		** memory.
		*/
		spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanExpandIntensity : __glSpanExpandIntensityNS;
		if (spanInfo->zeroFillAlpha) {
		    spanInfo->spanModifier[spanCount++]=__glSpanZeroFillAlpha;
		}
		break;
	    }
	}

	break;

      case __GL_SRCDST_FB:
	/*
	**  Read and modify a span.  RGBA spans are scaled when
	** this step is finished.
	*/
	switch(dstFormat) {
	  case GL_RGB:
	  case GL_RED:
	  case GL_GREEN:
	  case GL_BLUE:
	  case GL_LUMINANCE:
	  case GL_LUMINANCE_ALPHA:
	  case GL_ALPHA:
	  case GL_RGBA:
	  case GL_ABGR_EXT:
	  case GL_BGR_EXT:
	  case GL_BGRA_EXT:
	  case GL_INTENSITY:
	  case __GL_RED_ALPHA:
	    if (gc->modes.rgbMode) {
		if (zoomx2) {
		    reader = gc->procs.pixel.spanReadRGBA2;
		} else {
		    reader = gc->procs.pixel.spanReadRGBA;
		}
		if (pm->modifyRGBA) {
		    spanInfo->spanModifier[spanCount++]=__glSpanPreUnscaleRGBA;
		    spanInfo->spanModifier[spanCount++]=__glSpanModifyRGBA;
		}
	    } else {
		if (zoomx2) {
		    reader = gc->procs.pixel.spanReadCI2;
		} else {
		    reader = gc->procs.pixel.spanReadCI;
		}
		spanInfo->spanModifier[spanCount++] = __glSpanModifyCI;
	    }
	    break;
	  case GL_DEPTH_COMPONENT:
	    if (zoomx2) {
		reader = gc->procs.pixel.spanReadDepth2;
	    } else {
		reader = gc->procs.pixel.spanReadDepth;
	    }
	    if (pm->modifyDepth) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyDepth;
	    }
	    break;
	  case GL_STENCIL_INDEX:
	    if (zoomx2) {
		reader = gc->procs.pixel.spanReadStencil2;
	    } else {
		reader = gc->procs.pixel.spanReadStencil;
	    }
	    if (pm->modifyStencil) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyStencil;
	    }
	    break;
	  case GL_COLOR_INDEX:
	    if (zoomx2) {
		reader = gc->procs.pixel.spanReadCI2;
	    } else {
		reader = gc->procs.pixel.spanReadCI;
	    }
	    if (pm->modifyCI) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyCI;
	    } 
	    break;
	  default:
	    assert(0);
	}
	/*
	** Optimization attempt for ReadPixels.
	**
	** This was taken from the DrawPixels path and modified
	** so the optimizations for DrawPixels will be simetrical
	** to the optimizations for ReadPixels.
	**
	** There are some format, type combinations that are expected to be 
	** common.  This code optimizes a few of those cases.  Specifically,
	** these modes include:  (GL_UNSIGNED_BYTE, GL_RGB), 
	** (GL_UNSIGNED_BYTE, GL_RGBA), (GL_UNSIGNED_BYTE, GL_COLOR_INDEX),
	** (GL_UNSIGNED_BYTE, GL_STENCIL_INDEX), 
	** (GL_UNSIGNED_SHORT, GL_COLOR_INDEX), 
	** (GL_UNSIGNED_SHORT, GL_STENCIL_INDEX),
	** (GL_UNSIGNED_INT, GL_DEPTH_COMPONENT)
	*/

	/* 
	** XXX have simpleXferPath flag for this logic.
	*/
	if(spanModInfo->pixelPath == __GL_PIXPATH_READPIX) {
	     /* XXX need the path check?? */
	    switch(dstType) {
	    case GL_UNSIGNED_BYTE:
		switch(dstFormat) {
		case GL_RGB:
		    break;
		case GL_RGBA:
		    break;
		case GL_STENCIL_INDEX:
		    break;
		case GL_COLOR_INDEX:
		    break;
		default:
		    break;
		}
		break;
	    case GL_UNSIGNED_SHORT:
		switch(dstFormat) {
		case GL_STENCIL_INDEX:
		    break;
		case GL_COLOR_INDEX:
		    break;
		default:
		    break;
		}
		break;
	    case GL_UNSIGNED_INT:
		switch(dstFormat) {
		case GL_DEPTH_COMPONENT:
		    if (!pm->modifyDepth) {
			GLuint scale = gc->depthBuffer.scale 
			    >> gc->depthBuffer.numFracBits;

			/*
			 ** we will be not converting to float if:
			 **  - scale will be of the form 2^n -1 after the
			 **    shift imposed by numFracBits
			 */
			if ( (scale & (scale+1)) == 0x00000000 ) {

			    /* Back off conversion to float */
			    assert(dstConvert);
			    spanCount--;

			    reader = __glSpanReadDepthUint2;
			}
		    }
		    break;
		default:
		    break;
		}
		break;
	    default:
		break;
	    }
	}
	break;

      default:
	assert(0);
    } /* end switch on srcType */

    /* end internal format; begin conversion to dst */

    switch(spanModInfo->dstType) {
      case __GL_SRCDST_FB:

	/*
	**  Rendering
	**
	** The spans are rendered.  If |xzoom| > 1, then the span renderer
	** is responsible for pixel replication.
	*/

	switch(dstFormat) {
	  case GL_RGBA:
	  case GL_RGB:
	  case GL_RED:
	  case GL_GREEN:
	  case GL_BLUE:
	  case GL_ALPHA:
	  case GL_LUMINANCE:
	  case GL_LUMINANCE_ALPHA:
	  case GL_ABGR_EXT:
	  case GL_BGR_EXT:
	  case GL_BGRA_EXT:
	  case GL_INTENSITY:
	    if (zoomx1) {
		render = gc->procs.pixel.spanRenderRGBA2;
	    } else {
		render = gc->procs.pixel.spanRenderRGBA;
	    }
	    break;
	  case GL_DEPTH_COMPONENT:
	    if (zoomx1) {
		render = gc->procs.pixel.spanRenderDepth2;
	    } else {
		render = gc->procs.pixel.spanRenderDepth;
	    }
	    break;
	  case GL_COLOR_INDEX:
	    if (zoomx1) {
		render = gc->procs.pixel.spanRenderCI2;
	    } else {
		render = gc->procs.pixel.spanRenderCI;
	    }
	    break;
	  case GL_STENCIL_INDEX:
	    if (zoomx1) {
		render = gc->procs.pixel.spanRenderStencil2;
	    } else {
		render = gc->procs.pixel.spanRenderStencil;
	    }
	    break;
	  default:
	    assert(0);
	}

	/*
	** Optimization attempt for DrawPixels.
	**
	** There are some format, type combinations that are expected to be 
	** common.  This code optimizes a few of those cases.  Specifically,
	** these modes include:  (GL_UNSIGNED_BYTE, GL_RGB), 
	** (GL_UNSIGNED_BYTE, GL_RGBA), (GL_UNSIGNED_BYTE, GL_COLOR_INDEX),
	** (GL_UNSIGNED_BYTE, GL_STENCIL_INDEX), 
	** (GL_UNSIGNED_SHORT, GL_COLOR_INDEX), 
	** (GL_UNSIGNED_SHORT, GL_STENCIL_INDEX),
	** (GL_UNSIGNED_INT, GL_DEPTH_COMPONENT)
	** (GL_UNSIGNED_INT, GL_COLOR_INDEX)
	*/

	if(spanModInfo->pixelPath == __GL_PIXPATH_DRAWPIX) {
	    switch(srcType) {
	    case GL_UNSIGNED_BYTE:
		switch(srcFormat) {
		case GL_RGB:
		    spanCount = 0;
		    if (packedUserData) {
			/* no span unpacking is necessary! */
		    } else {
			/* zoomx2 must not be true, or packedUserData would be set 
			 */
			assert(!zoomx2);
			spanInfo->spanModifier[spanCount++] = 
			    __glSpanUnpackRGBubyte;
		    }
		    if (!pm->modifyRGBA) {
			pm->redCurMap = pm->redMap;
			pm->greenCurMap = pm->greenMap;
			pm->blueCurMap = pm->blueMap;
			pm->alphaCurMap = pm->alphaMap;
			if (zoomx1) {
			    render = __glSpanRenderRGBubyte2;
			} else {
			    render = __glSpanRenderRGBubyte;
			}
		    } else {
			if (!pm->rgbaCurrent) {
			    __glBuildRGBAModifyTables(gc, pm);
			}
			pm->redCurMap = pm->redModMap;
			pm->greenCurMap = pm->greenModMap;
			pm->blueCurMap = pm->blueModMap;
			pm->alphaCurMap = pm->alphaModMap;
			if (zoomx1) {
			    render = __glSpanRenderRGBubyte2;
			} else {
			    render = __glSpanRenderRGBubyte;
			}
		    }
		    break;
		case GL_RGBA:
		    spanCount = 0;
		    if (packedUserData) {
			/* no span unpacking is necessary! */
		    } else {
			/* zoomx2 must not be true, or packedUserData would be set 
			 */
			assert(!zoomx2);
			spanInfo->spanModifier[spanCount++] = 
			    __glSpanUnpackRGBAubyte;
		    }
		    if (!pm->modifyRGBA) {
			pm->redCurMap = pm->redMap;
			pm->greenCurMap = pm->greenMap;
			pm->blueCurMap = pm->blueMap;
			pm->alphaCurMap = pm->alphaMap;
		    } else {
			if (!pm->rgbaCurrent) {
			    __glBuildRGBAModifyTables(gc, pm);
			}
			pm->redCurMap = pm->redModMap;
			pm->greenCurMap = pm->greenModMap;
			pm->blueCurMap = pm->blueModMap;
			pm->alphaCurMap = pm->alphaModMap;
		    }
		    if (zoomx1) {
			render = __glSpanRenderRGBAubyte2;
		    } else {
			render = __glSpanRenderRGBAubyte;
		    }
		    break;
		case GL_STENCIL_INDEX:
		    if (!pm->modifyStencil) {
			spanCount = 0;
			if (packedUserData) {
			    /* no span unpacking is necessary! */
			} else {
			    /* zoomx2 must not be true, or packedUserData 
			     * would be set 
			     */
			    assert(!zoomx2);
			    spanInfo->spanModifier[spanCount++] = 
				__glSpanUnpackIndexUbyte;
			}
			if (zoomx1) {
			    render = __glSpanRenderStencilUbyte2;
			} else {
			    render = __glSpanRenderStencilUbyte;
			}
		    }
		    break;
		case GL_COLOR_INDEX:
		    spanCount = 0;
		    if (packedUserData) {
			/* no span unpacking is necessary! */
		    } else {
			/* zoomx2 must not be true, or packedUserData would be set 
			 */
			assert(!zoomx2);
			spanInfo->spanModifier[spanCount++] = 
			    __glSpanUnpackIndexUbyte;
		    }
		    if (!pm->modifyCI) {
			pm->iCurMap = pm->iMap;
			if (zoomx1) {
			    render = __glSpanRenderCIubyte2;
			} else {
			    render = __glSpanRenderCIubyte;
			}
		    } else {
			if (gc->modes.rgbMode) {
			    if (!pm->iToRGBACurrent) {
				__glBuildItoRGBAModifyTables(gc, pm);
			    }
			    pm->redCurMap = pm->iToRMap;
			    pm->greenCurMap = pm->iToGMap;
			    pm->blueCurMap = pm->iToBMap;
			    pm->alphaCurMap = pm->iToAMap;
			    if (zoomx1) {
				render = __glSpanRenderCIubyte4;
			    } else {
				render = __glSpanRenderCIubyte3;
			    }
			} else {
			    if (!pm->iToICurrent) {
				__glBuildItoIModifyTables(gc, pm);
			    }
			    pm->iCurMap = pm->iToIMap;
			    if (zoomx1) {
				render = __glSpanRenderCIubyte2;
			    } else {
				render = __glSpanRenderCIubyte;
			    }
			}
		    }
		    break;
		default:
		    break;
		}
		break;
	    case GL_UNSIGNED_SHORT:
		switch(srcFormat) {
		case GL_STENCIL_INDEX:
		    if (!pm->modifyStencil) {
			/* Back off conversion to float */
			assert(srcConvert);
			spanCount--;
			if (zoomx1) {
			    render = __glSpanRenderStencilUshort2;
			} else {
			    render = __glSpanRenderStencilUshort;
			}
		    }
		    break;
		case GL_COLOR_INDEX:
		    if (!pm->modifyCI) {
			/* Back off conversion to float */
			assert(srcConvert);
			spanCount--;
			if (zoomx1) {
			    render = __glSpanRenderCIushort2;
			} else {
			    render = __glSpanRenderCIushort;
			}
		    }
		    break;
		default:
		    break;
		}
		break;
	    case GL_UNSIGNED_INT:
		switch(srcFormat) {
		case GL_DEPTH_COMPONENT:
		    if (!pm->modifyDepth) {
			GLuint writeMask = gc->depthBuffer.writeMask;
			GLuint scale = gc->depthBuffer.scale 
			    >> gc->depthBuffer.numFracBits;

			/*
			 ** we will be not converting to float if:
			 **  - writeMask is of the form 2^n -1
			 **  - scale will be of the form 2^n -1 after the
			 **    shift imposed by numFracBits
			 */
			if( ( (writeMask & (writeMask+1)) == 0x00000000 ) &
			   ( (scale & (scale+1)) == 0x00000000 ) ) {

			    /* Back off conversion to float */
			    assert(srcConvert);
			    spanCount--;

			    if (zoomx1) {
				render = __glSpanRenderDepthUint2;
			    } else {
				render = __glSpanRenderDepthUint;
			    }
			}
		    }
		    break;
		case GL_COLOR_INDEX:
		    if (!pm->modifyCI) {
			/* Back off conversion to float */
			assert(srcConvert);
			spanCount--;
			if (zoomx1) {
			    render = __glSpanRenderCIuint2;
			} else {
			    render = __glSpanRenderCIuint;
			}
		    }
		    break;
		default:
		    break;
		}
		break;
	    default:
		break;
	    }
	}
	break;

      case __GL_SRCDST_MEM:
	/*
	**  Reduce RGBA spans to appropriate derivative (RED, 
	** LUMINANCE, ALPHA, etc.).
	*/
	if (dstReduce) {
		switch(dstFormat) {
		  case GL_RGB:
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanReduceRGB : __glSpanReduceRGBNS;
		    break;
		  case GL_BGR_EXT:
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanReduceBGR : __glSpanReduceBGRNS;
		    break;
		  case GL_RED:
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanReduceRed : __glSpanReduceRedNS;
		    break;
		  case GL_GREEN:
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanReduceGreen : __glSpanReduceGreenNS;
		    break;
		  case GL_BLUE:
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanReduceBlue : __glSpanReduceBlueNS;
		    break;
		  case GL_LUMINANCE:
		    if (spanModInfo->pixelPath == __GL_PIXPATH_READPIX) {
			spanInfo->spanModifier[spanCount++] = 
			    spanInfo->applyFbScale ? 
			    __glSpanReduceLuminance : 
			    __glSpanReduceRedNS;
		    } else {
			spanInfo->spanModifier[spanCount++] = 
			    spanInfo->applyFbScale ? 
			    __glSpanReduceRed : __glSpanReduceRedNS;
		    }
		    break;
		  case GL_LUMINANCE_ALPHA:
		    if (spanModInfo->pixelPath == __GL_PIXPATH_READPIX) {
			spanInfo->spanModifier[spanCount++] = 
			    spanInfo->applyFbScale ? 
			    __glSpanReduceLuminanceAlpha : 
			    __glSpanReduceRedAlphaNS;
		    } else {
			spanInfo->spanModifier[spanCount++] = 
			    spanInfo->applyFbScale ? 
			    __glSpanReduceRedAlpha : 
			    __glSpanReduceRedAlphaNS;
		    }
		    break;
		  case __GL_RED_ALPHA:
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanReduceRedAlpha : __glSpanReduceRedAlphaNS;
		    break;
		  case GL_ALPHA:
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanReduceAlpha : __glSpanReduceAlphaNS;
		    break;
		  case GL_RGBA:
		    if (spanInfo->applyFbScale) {
			spanInfo->spanModifier[spanCount++] = 
						__glSpanPostUnscaleRGBA;
		    }
		    break;
		  case GL_ABGR_EXT:
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanReorderABGR : __glSpanReorderABGRNS;
		    break;
		  case GL_BGRA_EXT:
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanReorderBGRA : __glSpanReorderBGRANS;
		    break;
		  case GL_INTENSITY:
		    spanInfo->spanModifier[spanCount++] = 
			spanInfo->applyFbScale ? 
			__glSpanReduceRed : __glSpanReduceRedNS;
		    break;
		}
	}

	/*
	** Clamp if necessary.
	*/
	if (dstClamp) {
	    switch(srcType) {
	      case GL_BYTE:
	      case GL_SHORT:
	      case GL_INT:
		spanInfo->spanModifier[spanCount++] = __glSpanClampSigned;
		break;
	      case GL_FLOAT:
		spanInfo->spanModifier[spanCount++] = __glSpanClampPostFloat;
		break;
	    }
	}

	/*
	**  Conversion from FLOAT to user requested type.
	*/
	if (dstConvert) {
	    if (isIndex) {
		switch(dstType) {
		  case GL_BYTE:
		    spanInfo->spanModifier[spanCount++] = __glSpanPackByteI;
		    break;
		  case GL_UNSIGNED_BYTE:
		    spanInfo->spanModifier[spanCount++] = __glSpanPackUbyteI;
		    break;
		  case GL_SHORT:
		    spanInfo->spanModifier[spanCount++] = __glSpanPackShortI;
		    break;
		  case GL_UNSIGNED_SHORT:
		    spanInfo->spanModifier[spanCount++] = __glSpanPackUshortI;
		    break;
		  case GL_INT:
		    spanInfo->spanModifier[spanCount++] = __glSpanPackIntI;
		    break;
		  case GL_UNSIGNED_INT:
		    spanInfo->spanModifier[spanCount++] = __glSpanPackUintI;
		    break;
		  case GL_BITMAP:
		    spanInfo->spanModifier[spanCount++] = __glSpanPackBitmap;
		    break;
		}
	    } else {
		switch(dstType) {
		  case GL_BYTE:
		    spanInfo->spanModifier[spanCount++] = __glSpanPackByte;
		    break;
		  case GL_UNSIGNED_BYTE:
		    spanInfo->spanModifier[spanCount++] = __glSpanPackUbyte;
		    break;
		  case GL_SHORT:
		    spanInfo->spanModifier[spanCount++] = __glSpanPackShort;
		    break;
		  case GL_UNSIGNED_SHORT:
		    spanInfo->spanModifier[spanCount++] = __glSpanPackUshort;
		    break;
		  case GL_INT:
		    spanInfo->spanModifier[spanCount++] = __glSpanPackInt;
		    break;
		  case GL_UNSIGNED_INT:
		    spanInfo->spanModifier[spanCount++] = __glSpanPackUint;
		    break;
		  case GL_UNSIGNED_BYTE_3_3_2_EXT:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanPack332Ubyte;
		    spanInfo->dstComponents = 3;
		    break;
		  case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanPack4444Ushort;
		    spanInfo->dstComponents = 4;
		    break;
		  case __GL_UNSIGNED_SHORT_4_4_4_4_ARGB:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanPackARGB4444Ushort;
		    spanInfo->dstComponents = 4;
		    break;
		  case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanPack5551Ushort;
		    spanInfo->dstComponents = 4;
		    break;
		  case __GL_UNSIGNED_SHORT_X_5_5_5:
		    spanInfo->spanModifier[spanCount++] =
						__glSpanPackX555Ushort;
		    spanInfo->dstComponents = 3;
		    break;
		  case __GL_UNSIGNED_SHORT_1_5_5_5:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanPack1555Ushort;
		    spanInfo->dstComponents = 4;
		    break;
		  case __GL_UNSIGNED_SHORT_5_6_5:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanPack565Ushort;
		    spanInfo->dstComponents = 3;
		    break;
		  case GL_UNSIGNED_INT_8_8_8_8_EXT:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanPack8888Uint;
		    spanInfo->dstComponents = 4;
		    break;
		  case GL_UNSIGNED_INT_10_10_10_2_EXT:
		    spanInfo->spanModifier[spanCount++] = 
						__glSpanPack_10_10_10_2_Uint;
		    spanInfo->dstComponents = 4;
		    break;
		  default:
		    assert(0);
		}
	    }
	} else if (spanInfo->nonColorComp) {
	    /* only needed for hgram so far; these are the hgram dest types */
	    switch(dstType) {
	    case GL_BYTE:
		spanInfo->spanModifier[spanCount++] = __glSpanPackNonCompByte;
		break;
	    case GL_UNSIGNED_BYTE:
		spanInfo->spanModifier[spanCount++] = __glSpanPackNonCompUbyte;
		break;
	    case GL_SHORT:
		spanInfo->spanModifier[spanCount++] = __glSpanPackNonCompShort;
		break;
	    case GL_UNSIGNED_SHORT:
		spanInfo->spanModifier[spanCount++] = __glSpanPackNonCompUshort;
		break;
	    case GL_INT:
		spanInfo->spanModifier[spanCount++] = __glSpanPackNonCompInt;
		break;
	    case GL_UNSIGNED_INT:
		spanInfo->spanModifier[spanCount++] = __glSpanPackNonCompUint;
		break;
	    }
	}

	/*
	**  Mis-align data as needed, and perform byte swapping
	** if requested by the user.
	*/
	if (dstSwap) {
	    if (spanInfo->dstElementSize == 2) {
		spanInfo->spanModifier[spanCount++] = __glSpanSwapBytes2Dst;
	    } else if (spanInfo->dstElementSize == 4) {
		spanInfo->spanModifier[spanCount++] = __glSpanSwapBytes4Dst;
	    }
	} else if (dstAlign) {
	    if (spanInfo->dstElementSize == 2) {
		spanInfo->spanModifier[spanCount++] = __glSpanAlignPixels2Dst;
	    } else if (spanInfo->dstElementSize == 4) {
		spanInfo->spanModifier[spanCount++] = __glSpanAlignPixels4Dst;
	    }
	}
	break;

      default:
	assert(0);
	break;
    } /* end switch on dstType */

done:
    spanModInfo->reader = reader;
    spanModInfo->render = render;

    spanInfo->numSpanMods = spanCount;
}



/*
** This routine clips a draw pixels box, and sets up a bunch of 
** variables required for drawing the box.  These are some of them:
**
** startCol   - The first column that will be drawn.
** x          - Effective raster position.  This will be set up so that 
**		every time zoomx is added, a change in the integer portion
**		of x indicates that a pixel should rendered (unpacked).
** columns    - The total number of columns that will be rendered.
**
** Others are startRow, y, rows.
**
** Yet other variables may be modified, such as width, height, skipPixels,
** skipLines.
**
** The clipping routine is written very carefully so that a fragment will
** be rasterized by a pixel if it's center falls within the range
** [x, x+zoomx) x [y, y+zoomy).
*/
GLboolean __glClipDrawPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint skipPixels;
    GLint skipRows;
    GLint width, height;
    GLint tempint;
    GLint endCol, endRow;
    __GLfloat x,y,x2,y2;
    __GLfloat zoomx, zoomy;
    __GLfloat clipLeft, clipRight, clipBottom, clipTop;

    zoomx = spanInfo->zoomx;
    zoomy = spanInfo->zoomy;
    if (zoomx == __glZero || zoomy == __glZero) {
	return GL_FALSE;
    }

    skipPixels = skipRows = 0;
    width = spanInfo->width;
    height = spanInfo->height;
    clipLeft = gc->transform.clipX0 + __glHalf;
    clipBottom = gc->transform.clipY0 + __glHalf;
    clipRight = gc->transform.clipX1 - gc->constants.viewportAlmostHalf;
    clipTop = gc->transform.clipY1 - gc->constants.viewportAlmostHalf;

#if defined(__GL_HALF_PIXEL_OFFSET)
    clipLeft -= __glHalf;
    clipBottom -= __glHalf;
    clipRight -= __glHalf;
    clipTop -= __glHalf;
#endif

    x = spanInfo->x;
    y = spanInfo->y;
    x2 = x + zoomx * width;
    y2 = y + zoomy * height;

    if (zoomx > 0) {
	/* Zoomx is positive, clip the left edge */
	if (x > clipLeft) {
	    /* Clip to the first fragment that will be produced */
	    clipLeft = (GLint) (x + gc->constants.viewportAlmostHalf);
	    clipLeft += __glHalf;
	}
	skipPixels = (clipLeft-x) / zoomx;
	if (skipPixels >= width) return GL_FALSE;

	width -= skipPixels;
	spanInfo->startCol = clipLeft;
	x = x + skipPixels * zoomx;
	spanInfo->x = x + gc->constants.viewportAlmostHalf;
	spanInfo->srcSkipPixels += skipPixels;

	/* Zoomx is positive, clip the right edge */
	if (x2 < clipRight) {
	    /* Clip to the last fragment that will be produced */
	    clipRight = (GLint) (x2 + gc->constants.viewportAlmostHalf);
	    clipRight -= gc->constants.viewportAlmostHalf;
	}
	tempint = (x2-clipRight) / zoomx;
	if (tempint >= width) return GL_FALSE;

	width -= tempint;
	endCol = (GLint) clipRight + 1;
	spanInfo->endCol = endCol;
	spanInfo->columns = endCol - spanInfo->startCol;
    } else /* zoomx < 0 */ {
	/* Zoomx is negative, clip the right edge */
	if (x < clipRight) {
	    /* Clip to the first fragment that will be produced */
	    clipRight = (GLint) (x + gc->constants.viewportAlmostHalf);
	    clipRight -= gc->constants.viewportAlmostHalf;
	}
	skipPixels = (clipRight-x) / zoomx;
	if (skipPixels >= width) return GL_FALSE;

	width -= skipPixels;
	spanInfo->startCol = clipRight;
	x = x + skipPixels * zoomx;
	spanInfo->x = x + gc->constants.viewportAlmostHalf - __glOne;
	spanInfo->srcSkipPixels += skipPixels;

	/* Zoomx is negative, clip the left edge */
	if (x2 > clipLeft) {
	    clipLeft = (GLint) (x2 + gc->constants.viewportAlmostHalf);
	    clipLeft += __glHalf;
	}
	tempint = (x2-clipLeft) / zoomx;
	if (tempint >= width) return GL_FALSE;

	width -= tempint;
	endCol = (GLint) clipLeft - 1;
	spanInfo->endCol = endCol;
	spanInfo->columns = spanInfo->startCol - endCol;
    }

    if (zoomy > 0) {
	/* Zoomy is positive, clip the bottom edge */
	if (y > clipBottom) {
	    /* Clip to the first row that will be produced */
	    clipBottom = (GLint) (y + gc->constants.viewportAlmostHalf);
	    clipBottom += __glHalf;
	}
	skipRows = (clipBottom-y) / zoomy;
	if (skipRows >= height) return GL_FALSE;

	height -= skipRows;
	spanInfo->startRow = clipBottom;
	y = y + skipRows * zoomy;
	spanInfo->y = y + gc->constants.viewportAlmostHalf;
	spanInfo->srcSkipLines += skipRows;

	/* Zoomy is positive, clip the top edge */
	if (y2 < clipTop) {
	    /* Clip to the last row that will be produced */
	    clipTop = (GLint) (y2 + gc->constants.viewportAlmostHalf);
	    clipTop -= gc->constants.viewportAlmostHalf;
	}
	tempint = (y2-clipTop) / zoomy;
	if (tempint >= height) return GL_FALSE;

	height -= tempint;
	endRow = (GLint) clipTop + 1;
	spanInfo->rows = endRow - spanInfo->startRow;
    } else /* zoomy < 0 */ {
	/* Zoomy is negative, clip the top edge */
	if (y < clipTop) {
	    /* Clip to the first row that will be produced */
	    clipTop = (GLint) (y + gc->constants.viewportAlmostHalf);
	    clipTop -= gc->constants.viewportAlmostHalf;
	}
	skipRows = (clipTop-y) / zoomy;
	if (skipRows >= height) return GL_FALSE;

	height -= skipRows;
	spanInfo->startRow = clipTop;
	y = y + skipRows * zoomy;
	/* spanInfo->y = y - __glHalf; */
	spanInfo->y = y + gc->constants.viewportAlmostHalf - __glOne;
	spanInfo->srcSkipLines += skipRows;

	/* Zoomy is negative, clip the bottom edge */
	if (y2 > clipBottom) {
	    clipBottom = (GLint) (y2 + gc->constants.viewportAlmostHalf);
	    clipBottom += __glHalf;
	}
	tempint = (y2-clipBottom) / zoomy;
	if (tempint >= height) return GL_FALSE;

	height -= tempint;
	endRow = (GLint) clipBottom - 1;
	spanInfo->rows = spanInfo->startRow - endRow;
    }

    spanInfo->width = width;
    spanInfo->height = height;

    return GL_TRUE;
}

/*
** This routine computes spanInfo->pixelArray if needed.
**
** If |zoomx| > 1.0, this array contains counts for how many times to 
** replicate a given pixel.  For example, if zoomx is 2.0, this array will
** contain all 2's.  If zoomx is 1.5, then every other entry will contain 
** a 2, and every other entry will contain a 1.
**
** if |zoomx| < 1.0, this array contains counts for how many pixels to 
** skip.  For example, if zoomx is 0.5, every entry in the array will contain
** a 2 (indicating to skip forward two pixels [only past one]).  If zoomx is
** .666, then every other entry will be a 2, and every other entry will be 
** a 1.
*/
void __glComputeSpanPixelArray(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint width, intx;
    __GLfloat zoomx, oldx, newx;
    GLint i;
    GLshort *array;
    
    zoomx = spanInfo->zoomx;
    if (zoomx > (__GLfloat) -1.0 && zoomx < __glOne) {
	GLint lasti;

	/* Build pixel skip array */
	width = spanInfo->width;
	oldx = spanInfo->x;
	array = spanInfo->pixelArray;

	intx = (GLint) oldx;
	newx = oldx;

	lasti = 0;
	for (i=0; i<width; i++) {
	    /* Skip groups which will not be rasterized */
	    newx += zoomx;
	    while ((GLint) newx == intx && i<width) {
		newx += zoomx;
		i++;
	    }
	    assert (i != width);
	    if (i != 0) {
		*array++ = (GLshort) (i - lasti);
	    }
	    lasti = i;
	    intx = (GLint) newx;
	}
	*array++ = 1;
    } else if (zoomx < (__GLfloat) -1.0 || zoomx > __glOne) {
	__GLfloat right;
	GLint iright;
	GLint coladd, column;
	GLint startCol;

	/* Build pixel replication array */
	width = spanInfo->width - 1;
	startCol = spanInfo->startCol;
	column = startCol;
	coladd = spanInfo->coladd;
	array = spanInfo->pixelArray;
	right = spanInfo->x;
	for (i=0; i<width; i++) {
	    right = right + zoomx;
	    iright = right;
	    *array++ = (GLshort) (iright - column);
	    column = iright;
	}
	if (coladd == 1) {
	    *array++ = (GLshort) (spanInfo->columns - (column - startCol));
	} else {
	    *array++ = (GLshort) ((startCol - column) - spanInfo->columns);
	}
    }
}

/*
** Initialize the spanInfo structure.  If "packed" is true, the structure
** is initialized for unpacking data from a display list.  If "packed" is 
** false, it is initialized for unpacking data from the user's data space.
*/
void __glLoadUnpackModes(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			 GLboolean packed)
{

    if (packed) {
	/*
	** Data came from a display list.
	*/

	spanInfo->srcAlignment = 1;
	spanInfo->srcSkipPixels = 0;
	spanInfo->srcSkipLines = 0;
	spanInfo->srcLsbFirst = GL_FALSE;
	spanInfo->srcSwapBytes = GL_FALSE;
	spanInfo->srcLineLength = spanInfo->width;
    } else {
	GLint lineLength;

	/*
	** Data came straight from the application.
	*/

	lineLength = gc->state.pixel.unpackModes.lineLength;
	spanInfo->srcAlignment = gc->state.pixel.unpackModes.alignment;
	spanInfo->srcSkipPixels = gc->state.pixel.unpackModes.skipPixels;
	spanInfo->srcSkipLines = gc->state.pixel.unpackModes.skipLines;
	spanInfo->srcLsbFirst = gc->state.pixel.unpackModes.lsbFirst;
	spanInfo->srcSwapBytes = gc->state.pixel.unpackModes.swapEndian;
	if (lineLength <= 0) lineLength = spanInfo->width;
	spanInfo->srcLineLength = lineLength;
    }
}

void __glInitDrawPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		            GLint width, GLint height, GLenum format, 
			    GLenum type, const GLvoid *pixels)
{
    __GLfloat x,y;
    __GLfloat zoomx, zoomy;

    x = gc->state.current.rasterPos.window.x;
    y = gc->state.current.rasterPos.window.y;

#if defined(__GL_HALF_PIXEL_OFFSET)
    x -= __glHalf;
    y -= __glHalf;
#endif

    spanInfo->x = x;
    spanInfo->y = y;
    spanInfo->fragz = gc->state.current.rasterPos.window.z *
						gc->constants.depthRescale;
    zoomx = gc->state.pixel.transferMode.zoomX;
    if (zoomx > __glZero) {
	if (zoomx < __glOne) {
	    spanInfo->rendZoomx = __glOne;
	} else {
	    spanInfo->rendZoomx = zoomx;
	}
	spanInfo->coladd = 1;
    } else {
	if (zoomx > (GLfloat) -1.0) {
	    spanInfo->rendZoomx = (GLfloat) -1.0;
	} else {
	    spanInfo->rendZoomx = zoomx;
	}
	spanInfo->coladd = -1;
    }
    spanInfo->zoomx = zoomx;
    zoomy = gc->state.pixel.transferMode.zoomY;
    if (gc->constants.yInverted) {
	zoomy = -zoomy;
    } else {
	spanInfo->y += gc->constants.viewportEpsilon;
    }
    if (zoomy > __glZero) {
	spanInfo->rowadd = 1;
    } else {
	spanInfo->rowadd = -1;
    }
    spanInfo->zoomy = zoomy;
    spanInfo->width = width;
    spanInfo->height = height;
    if (format == GL_COLOR_INDEX && gc->modes.rgbMode) {
	spanInfo->dstFormat = GL_RGBA;
    } else {
	spanInfo->dstFormat = format;
    }
    spanInfo->srcFormat = format;
    spanInfo->srcType = type;
    spanInfo->srcImage = pixels;

    /*
    ** These aren't needed for DrawPixels, but they are read by the
    ** generic pixel path setup code, so something needs to be set
    ** in them.
    */
    spanInfo->dstType = GL_FLOAT;
    spanInfo->dstElementSize = 4;
    spanInfo->dstSwapBytes = GL_FALSE;
    spanInfo->dstImage = NULL;
    spanInfo->dim = 2;
}

void __glDrawPixelSpans(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    int i;
    __GLfloat zoomy, newy;
    GLint inty, height;
    GLint numSpanMods = spanInfo->numSpanMods;
    GLint spanModNum;
    void (**spanModifier)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
    GLubyte *tmpSpanData, *spanDataIn, *spanDataOut;

    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    spanModifier = spanInfo->spanModifier;
    render = spanInfo->spanRender;

    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;

    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->y = newy;
	    spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		    spanInfo->srcRowIncrement;
	    newy += zoomy;
	    i++;
	    assert (i != height);
	}
	inty = (GLint) newy;

	(*spanModifier[0])(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	spanDataIn = spanData1; spanDataOut = spanData2;
	for (spanModNum = 1; spanModNum < numSpanMods; spanModNum++) {
	    (*spanModifier[spanModNum])(gc, spanInfo, spanDataIn, spanDataOut);
	    tmpSpanData = spanDataIn;
	    spanDataIn = spanDataOut;
	    spanDataOut = tmpSpanData;
	}
	(*render)(gc, spanInfo, spanDataIn);
    }
}

/*
** This is the generic DrawPixels routine.  It applies two span modification
** routines followed by a span rendering routine.
*/
void __glDrawPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    int i;
    __GLfloat zoomy, newy;
    GLint inty, height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (*render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    render = spanInfo->spanRender;

    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;

    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->y = newy;
	    spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		    spanInfo->srcRowIncrement;
	    newy += zoomy;
	    i++;
	    assert (i != height);
	}
	inty = (GLint) newy;
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	(*span2)(gc, spanInfo, spanData1, spanData2);
	(*render)(gc, spanInfo, spanData2);
    }
}

/* 
** Draw pixels with only one span modification routine.
*/
void __glDrawPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    int i;
    __GLfloat zoomy, newy;
    GLint inty, height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (*render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    span1 = spanInfo->spanModifier[0];
    render = spanInfo->spanRender;

    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;

    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->y = newy;
	    spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		    spanInfo->srcRowIncrement;
	    newy += zoomy;
	    i++;
	    assert (i != height);
	}
	inty = (GLint) newy;
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	(*render)(gc, spanInfo, spanData1);
    }
}

/* 
** Draw pixels with no span modification routines.
*/
void __glDrawPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    int i;
    __GLfloat zoomy, newy;
    GLint inty, height;
    void (*render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    render = spanInfo->spanRender;

    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;

    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->y = newy;
	    spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		    spanInfo->srcRowIncrement;
	    newy += zoomy;
	    i++;
	    assert (i != height);
	}
	inty = (GLint) newy;
	(*render)(gc, spanInfo, spanInfo->srcCurrent);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
    }
}

/*
** Generic implementation of a DrawPixels picker.  Any machine specific
** implementation should provide their own.
*/
void __glSlowPickDrawPixels(__GLcontext *gc, GLint width, GLint height,
		            GLenum format, GLenum type, const GLvoid *pixels,
			    GLboolean packed)
{
    __GLpixelSpanInfo spanInfo;
    
    __glInitDrawPixelsInfo(gc, &spanInfo, width, height, format, type, pixels);
    __glLoadUnpackModes(gc, &spanInfo, packed);

    if(!__glClipDrawPixels(gc, &spanInfo)) return;

    __glInitUnpacker(gc, &spanInfo);

    __GL_LOCK_PXL_DRAW_BUFFER(gc, format);

    __glGenericPickDrawPixels(gc, &spanInfo);

    __GL_UNLOCK_PXL_DRAW_BUFFER(gc, format);
}

/*
** Generic picker for DrawPixels.  This should be called if no machine
** specific path is provided for this specific version of DrawPixels.
*/
void __glGenericPickDrawPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    void (*dpfn)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
    __GLpixelSpanModInfo spanModInfo;

    spanModInfo.srcType = __GL_SRCDST_MEM;
    spanModInfo.dstType = __GL_SRCDST_FB;
    spanModInfo.pixelPath = __GL_PIXPATH_DRAWPIX;
    spanModInfo.applyPixelTransfer = GL_TRUE;

    spanInfo->numSpanMods = 0;
    PickSpanModifiers(gc, spanInfo, &spanModInfo);

    spanInfo->spanRender = spanModInfo.render;

    /*
    ** Pick a DrawPixels function that applies the correct number of 
    ** span modifiers.
    */

    switch(spanInfo->numSpanMods) {
    case 0:
	dpfn = __glDrawPixels0;
	break;
    case 1:
	dpfn = __glDrawPixels1;
	break;
    case 2:
	dpfn = __glDrawPixels2;
	break;
    default:
	dpfn = __glDrawPixelSpans;
	break;
    }

    (*dpfn)(gc, spanInfo);
}

/*
** This routine clips ReadPixels calls so that only fragments which are
** owned by this context will be read and copied into the user's data.
** Parts of the ReadPixels rectangle lying outside of the window will
** be ignored.
*/
GLboolean __glClipReadPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint clipLeft, clipRight, clipTop, clipBottom;
    GLint x,y,x2,y2;
    GLint skipPixels, skipRows;
    GLint width, height;
    GLint tempint;

    width = spanInfo->width;
    height = spanInfo->height;
    x = spanInfo->readX;
    y = spanInfo->readY;
    x2 = x + spanInfo->width;
    if (gc->constants.yInverted) {
	y2 = y - spanInfo->height;
    } else {
	y2 = y + spanInfo->height;
    }
    clipLeft = gc->constants.viewportXAdjust;
    clipRight = gc->constants.width + gc->constants.viewportXAdjust;
    clipBottom = gc->constants.viewportYAdjust;
    clipTop = gc->constants.height + gc->constants.viewportYAdjust;
    skipPixels = 0;
    skipRows = 0;
    if (x < clipLeft) {
	skipPixels = clipLeft - x;
	if (skipPixels > width) return GL_FALSE;

	width -= skipPixels;
	x = clipLeft;
	spanInfo->dstSkipPixels += skipPixels;
	spanInfo->readX = x;
    }
    if (x2 > clipRight) {
	tempint = x2 - clipRight;
	if (tempint > width) return GL_FALSE;

	width -= tempint;
    }
    if (gc->constants.yInverted) {
	if (y >= clipTop) {
	    skipRows = y - clipTop + 1;
	    if (skipRows > height) return GL_FALSE;

	    height -= skipRows;
	    y = clipTop - 1;
	    spanInfo->dstSkipLines += skipRows;
	    spanInfo->readY = y;
	}
	if (y2 < clipBottom - 1) {
	    tempint = clipBottom - y2 - 1;
	    if (tempint > height) return GL_FALSE;

	    height -= tempint;
	}
    } else {
	if (y < clipBottom) {
	    skipRows = clipBottom - y;
	    if (skipRows > height) return GL_FALSE;

	    height -= skipRows;
	    y = clipBottom;
	    spanInfo->dstSkipLines += skipRows;
	    spanInfo->readY = y;
	}
	if (y2 > clipTop) {
	    tempint = y2 - clipTop;
	    if (tempint > height) return GL_FALSE;

	    height -= tempint;
	}
    }

    spanInfo->width = width;
    spanInfo->height = height;

    return GL_TRUE;
}

/*
** Initialize the spanInfo structure for packing data into the user's data
** space.
*/
void __glLoadPackModes(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint lineLength = gc->state.pixel.packModes.lineLength;

    spanInfo->dstAlignment = gc->state.pixel.packModes.alignment;
    spanInfo->dstSkipPixels = gc->state.pixel.packModes.skipPixels;
    spanInfo->dstSkipLines = gc->state.pixel.packModes.skipLines;
    spanInfo->dstLsbFirst = gc->state.pixel.packModes.lsbFirst;
    spanInfo->dstSwapBytes = gc->state.pixel.packModes.swapEndian;
    if (lineLength <= 0) lineLength = spanInfo->width;
    spanInfo->dstLineLength = lineLength;
}

void __glInitReadPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		            GLint x, GLint y, GLint width, GLint height, 
			    GLenum format, GLenum type, const GLvoid *pixels)
{
    spanInfo->readX = x + gc->constants.viewportXAdjust;
    if (gc->constants.yInverted) {
	spanInfo->readY = (gc->constants.height - y - 1) + 
		gc->constants.viewportYAdjust;
    } else {
	spanInfo->readY = y + gc->constants.viewportYAdjust;
    }
    spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->dstFormat = format;
    spanInfo->dstType = type;
    spanInfo->dstImage = pixels;
    spanInfo->zoomx = __glOne;
    spanInfo->x = __glZero;
    __glLoadPackModes(gc, spanInfo);
    /*
    ** These aren't needed for ReadPixels, but they are read by the
    ** generic pixel path setup code, so something needs to be set
    ** in them.
    */
    spanInfo->srcType = GL_FLOAT;
    spanInfo->srcElementSize = 4;
    spanInfo->srcSwapBytes = GL_FALSE;
    spanInfo->srcImage = NULL;
    spanInfo->srcPackedData = GL_FALSE;
    spanInfo->srcFormat = gc->modes.rgbMode ? GL_RGBA : GL_COLOR_INDEX;
    spanInfo->dim = 2;
}

void __glReadPixelSpans(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i, ySign;
    GLint height;
    GLint numSpanMods = spanInfo->numSpanMods;
    GLint spanModNum;
    void (**spanModifier)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			      GLvoid *inspan, GLvoid *outspan);
    void (*reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			       GLvoid *outspan);
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
    GLubyte *tmpSpanData, *spanDataIn, *spanDataOut;

    reader = spanInfo->spanReader;

    ySign = gc->constants.ySign;
    height = spanInfo->height;
    spanModifier = spanInfo->spanModifier;

    for (i=0; i<height; i++) {
	(*reader)(gc, spanInfo, spanData1);
	spanDataIn = spanData1; spanDataOut = spanData2;
	for (spanModNum = 0; spanModNum < numSpanMods-1; spanModNum++) {
	    (*spanModifier[spanModNum])(gc, spanInfo, spanDataIn, spanDataOut);
	    tmpSpanData = spanDataIn;
	    spanDataIn = spanDataOut;
	    spanDataOut = tmpSpanData;
	}
	(*spanModifier[spanModNum])(gc, spanInfo, spanDataIn, spanInfo->dstCurrent);

	spanInfo->dstCurrent = (GLvoid *) ((GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement);
	spanInfo->readY += ySign;
    }
}

/*
** A simple generic ReadPixels routine with two span modifiers.
*/
void __glReadPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i, ySign;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (*reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];

    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    reader = spanInfo->spanReader;

    ySign = gc->constants.ySign;
    height = spanInfo->height;

    for (i=0; i<height; i++) {
	(*reader)(gc, spanInfo, spanData1);
	(*span1)(gc, spanInfo, spanData1, spanData2);
	(*span2)(gc, spanInfo, spanData2, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLvoid *) ((GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement);
	spanInfo->readY += ySign;
    }
}

/*
** A simple generic ReadPixels routine with one span modifier.
*/
void __glReadPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i, ySign;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];

    span1 = spanInfo->spanModifier[0];
    reader = spanInfo->spanReader;

    ySign = gc->constants.ySign;
    height = spanInfo->height;

    for (i=0; i<height; i++) {
	(*reader)(gc, spanInfo, spanData1);
	(*span1)(gc, spanInfo, spanData1, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLvoid *) ((GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement);
	spanInfo->readY += ySign;
    }
}

/*
** A simple generic ReadPixels routine with no span modifiers.
*/
void __glReadPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i, ySign;
    GLint height;
    void (*reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);

    reader = spanInfo->spanReader;

    ySign = gc->constants.ySign;
    height = spanInfo->height;

    for (i=0; i<height; i++) {
	(*reader)(gc, spanInfo, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLvoid *) ((GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement);
	spanInfo->readY += ySign;
    }
}

/*
** Generic implementation of a ReadPixels picker.  Any machine specific
** implementation should provide their own.
*/
void __glSlowPickReadPixels(__GLcontext *gc, GLint x, GLint y,
		            GLsizei width, GLsizei height,
		            GLenum format, GLenum type, const GLvoid *pixels)
{
    __GLpixelSpanInfo spanInfo;

    __glInitReadPixelsInfo(gc, &spanInfo, x, y, width, height, format, 
	    type, pixels);
    if (!__glClipReadPixels(gc, &spanInfo)) return;

    __glInitPacker(gc, &spanInfo);

    __GL_LOCK_PXL_READ_BUFFER(gc, format);

    __glGenericPickReadPixels(gc, &spanInfo);

    __GL_UNLOCK_PXL_READ_BUFFER(gc, format);
}

/*
** Generic picker for ReadPixels.  This should be called if no machine
** specific path is provided for this specific version of ReadPixels.
*/
void __glGenericPickReadPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    void (*rpfn)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
    __GLpixelSpanModInfo spanModInfo;

    spanModInfo.srcType = __GL_SRCDST_FB;
    spanModInfo.dstType = __GL_SRCDST_MEM;
    spanModInfo.pixelPath = __GL_PIXPATH_READPIX;
    spanModInfo.applyPixelTransfer = GL_TRUE;

    spanInfo->numSpanMods = 0;
    PickSpanModifiers(gc, spanInfo, &spanModInfo);
    spanInfo->spanReader = spanModInfo.reader;

    /*
    ** Pick a ReadPixels routine that uses the right number of span 
    ** modifiers.
    */

    switch(spanInfo->numSpanMods) {
    case 0:
	rpfn = __glReadPixels0;
	break;
    case 1:
	rpfn = __glReadPixels1;
	break;
    case 2:
	rpfn = __glReadPixels2;
	break;
    default:
	rpfn = __glReadPixelSpans;
	break;
    }

    (*rpfn)(gc, spanInfo);
}

/*
** This routine does two clips.  It clips like the DrawPixel clipper so 
** that if you try to copy to off window pixels, nothing will be done, and it 
** also clips like the ReadPixel clipper so that if you try to copy from
** off window pixels, nothing will be done.
*/
GLboolean __glClipCopyPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLfloat num, den;
    __GLfloat rpyUp, rpyDown;
    GLint rowsUp, rowsDown, startUp, startDown;
    __GLfloat midPoint;
    GLint intMidPoint, rowCount;
    GLint width, height;
    GLint readX, readY;
    __GLfloat zoomx, zoomy;
    __GLfloat rpx, rpy;
    __GLfloat rx1, rx2, ry1, ry2, wx1, wx2, wy1, wy2;
    __GLfloat abszoomy;
    GLint readUp, readDown;

    /*
    ** NOTE:
    ** A "nice" thing we could do for our application writers would be 
    ** to copy white when they try to copy from off window memory.  This
    ** would alert them to a bug in their program which they could then
    ** fix.
    **
    ** However, that seems like unnecessary code which would never be used
    ** anyway (no reason to bloat unnecessarily).
    */

    /*
    ** We take the easy approach, and just call the DrawPixels and ReadPixels
    ** clippers directly.
    */
    spanInfo->dstSkipLines = 0;
    spanInfo->dstSkipPixels = 0;
    if (!__glClipReadPixels(gc, spanInfo)) return GL_FALSE;
    spanInfo->x += spanInfo->dstSkipPixels * spanInfo->zoomx;
    spanInfo->y += spanInfo->dstSkipLines * spanInfo->zoomy;

    spanInfo->srcSkipLines = 0;
    spanInfo->srcSkipPixels = 0;
    if (!__glClipDrawPixels(gc, spanInfo)) return GL_FALSE;
    spanInfo->readX += spanInfo->srcSkipPixels;
    if (gc->constants.yInverted) {
	spanInfo->readY -= spanInfo->srcSkipLines;
    } else {
	spanInfo->readY += spanInfo->srcSkipLines;
    }

    /*
    ** Now for the incredibly tricky part!
    **
    ** This code attempts to deal with overlapping CopyPixels regions.
    ** It is a very difficult problem given that zoomy may be negative.
    ** The IrisGL used a cheap hack to solve this problem, which is 
    ** to read in the entire source image, and then write the destination
    ** image.  The problem with this approach, of course, is that it 
    ** requires a large amount of memory.
    **
    ** If zoomy can only be positive, then any image can be copied by
    ** copying a single span at a time, as long as you are careful about
    ** what order you process the spans.  However, since zoomy may be
    ** negative, the worst case images require copying two spans at 
    ** a time.  This means reading both spans, possibly modifying them,
    ** and then writing them back out. 
    **
    ** An example of this can be seen as follows:  Suppose an image
    ** covering 4 spans is copied onto itself with a zoomy of -1.  This
    ** means that the first row will be copied to the fourth row,
    ** and the fourth row will be copied to the first row.  In order 
    ** to accomplish both of these copies, they must be performed 
    ** simultaneously (after all, if you copy the first row to
    ** the fourth row first, then you have just destroyed the data 
    ** on the fourth row, and you can no longer copy it!).
    **
    ** In the most general case, any rectangular image can be copied
    ** by simultaneously iterating two spans over the source image
    ** and copying as you go.  Sometimes these spans will start at 
    ** the outside of the image and move their way inwards meeting 
    ** in the middle, and sometimes they will start in the middle 
    ** and work their way outward.
    **
    ** The middle point where the spans both start or end depends
    ** upon how the source and destination images overlap.  This point
    ** may be exactly in the middle, or at either end.  This means 
    ** that you may only end up with just a single span iterating over the 
    ** entire image (starting at one end and moving to the other).
    **
    ** The code that follows computes if the images overlap, and if they
    ** do, how two spans can be used to iterate over the source image
    ** so that it can be successfully copied to the destination image.
    **
    ** The following fields in the spanInfo record will be set in the 
    ** process of making these calculations:
    **
    ** overlap - set to GL_TRUE if the regions overlap at all.  Set to
    **		 GL_FALSE otherwise.
    **
    ** rowsUp, rowsDown - The number of rows of the source image that
    ** 			  need to be dealt with by the span that moves up
    **			  over the source image and the one that moves down
    **			  over the source image.  For example, if rowsUp is
    **			  equal to 10 and rowsDown is 0, then all 10 rows of 
    **			  the image should be copied by the up moving span
    **			  (the one that starts at readY and works it's way
    **			  up to readY+height).
    **
    ** startUp, startDown - At what relative points in time the spans should
    **			    start iterating.  For example, if startUp is 0
    **			    and startDown is 2, then the up moving span 
    **			    should be started first, and after it has 
    **			    iterated over 2 rows of the source image then
    **			    the down moving span should be started.
    **
    ** rpyUp, rpyDown - The starting raster positions for the two spans.
    **			These numbers are not exactly what they claim to
    **			be, but they are close.  They should be used by
    **			the span iterators in the following manner:  When
    **			the up moving span starts, it starts iterating 
    **			the float "rp_y" at rpyUp.  After reading and
    **			modifying a span, the span is written to rows
    **           	floor(rp_y) through floor(rp_y+zoomy) of the
    **			screen (not-inclusive of floor(rp_y+zoomy)).
    **			rp_y is then incremented by zoomy.  The same 
    **			algorithm is applied to the down moving span except
    **			that zoomy is subtracted from rp_y instead of
    **			being added.
    **
    ** readUp, readDown - The spans that are to be used for reading from
    **			  the source image.  The up moving span should start
    **			  reading at line "readUp", and the down moving span
    **			  should start at "readDown". 
    **
    ** Remember that the up moving and down moving spans must be iterated
    ** over the image simultaneously such that both spans are read before
    ** either one is written.
    **
    ** The actual algorithm applied here took many many hours of scratch 
    ** paper, and graph diagrams to derive.  It is very complicated, and
    ** hard to understand.  Do not attempt to change it without first
    ** understanding what it does completely.
    **
    ** In a nutshell, it first computes what span of the source image 
    ** will be copied onto itself (if any), and if |zoomy| < 1 it starts the
    ** up and down moving spans there and moves them outwards, or if 
    ** |zoomy| >= 1 it starts the spans at the outside of the image 
    ** and moves them inward so that they meet at the computed point.
    **
    ** Computing what span of the source image copies onto itself is 
    ** relatively easy.  For any span j of the source image from 0 through
    ** height, the span is read from row "readY + j" and written to
    ** any row centers falling within the range "rp_y + j * zoomy" through 
    ** "rp_y + (j+1) * zoomy".  If you set these equations equal to 
    ** each other (and subtract 0.5 from the raster position -- effectively
    ** moving the row centers from X.5 to X.0), you can determine that for 
    ** j = (readY - (rpy - 0.5)) / (zoomy-1) the source image concides with
    ** the destination image.  This is a floating point solution to a discrete
    ** problem, meaning that it is not a complete solution, but that is 
    ** the general idea.  Explaining this algorithm in any more detail would
    ** take another 1000 lines of comments, so I will leave it at that.
    */

    width = spanInfo->width;
    height = spanInfo->height;
    rpx = spanInfo->x;
    rpy = spanInfo->y;
    readX = spanInfo->readX;
    readY = spanInfo->readY;
    zoomx = spanInfo->zoomx;
    zoomy = spanInfo->zoomy;

    /* First check if the regions overlap at all */
    if (gc->constants.yInverted) {
	ry1 = readY - height + __glHalf;
	ry2 = readY - gc->constants.viewportAlmostHalf;
    } else {
	ry1 = readY + __glHalf;
	ry2 = readY + height - gc->constants.viewportAlmostHalf;
    }
    rx1 = readX + __glHalf;
    rx2 = readX + width - gc->constants.viewportAlmostHalf;
    if (zoomx > 0) {
	/* Undo some math done by ClipDrawPixels */
	rpx = rpx - gc->constants.viewportAlmostHalf;
	wx1 = rpx;
	wx2 = rpx + zoomx * width;
    } else {
	/* Undo some math done by ClipDrawPixels */
	rpx = rpx - gc->constants.viewportAlmostHalf + __glOne;
	wx1 = rpx + zoomx * width;
	wx2 = rpx;
    }
    if (zoomy > 0) {
	/* Undo some math done by ClipDrawPixels */
	rpy = rpy - gc->constants.viewportAlmostHalf;
	abszoomy = zoomy;
	wy1 = rpy;
	wy2 = rpy + zoomy * height;
    } else {
	/* Undo some math done by ClipDrawPixels */
	rpy = rpy - gc->constants.viewportAlmostHalf + __glOne;
	abszoomy = -zoomy;
	wy1 = rpy + zoomy * height;
	wy2 = rpy;
    }

    if (rx2 < wx1 || wx2 < rx1 || ry2 < wy1 || wy2 < ry1) {
	/* No overlap! */
	spanInfo->overlap = GL_FALSE;
	spanInfo->rowsUp = height;
	spanInfo->rowsDown = 0;
	spanInfo->startUp = 0;
	spanInfo->startDown = 0;
	spanInfo->rpyUp = rpy;
	spanInfo->rpyDown = rpy;
	return GL_TRUE;
    }

    spanInfo->overlap = GL_TRUE;

    /* Time to compute how we should set up our spans */
    if (gc->constants.yInverted) {
	num = (rpy - 0.5) - readY;
	den = -zoomy - 1;
    } else {
	num = readY - (rpy - 0.5);
	den = zoomy - 1;
    }
    startDown = startUp = 0;
    rowsUp = rowsDown = 0;
    rpyUp = rpy;
    rpyDown = rpy + zoomy*height;
    readUp = readY;
    if (gc->constants.yInverted) {
	readDown = readY - height + 1;
    } else {
	readDown = readY + height - 1;
    }

    if (den == __glZero) {
	/* Better not divide! */
	if (num > 0) {
	    midPoint = height;
	} else {
	    midPoint = 0;
	}
    } else {
	midPoint = num/den;
	if (midPoint < 0) {
	    midPoint = 0;
	} else if (midPoint > height) {
	    midPoint = height;
	}
    }
    if (midPoint == 0) {
	/* Only one span needed */
	if (abszoomy < __glOne) {
	    rowsUp = height;
	} else {
	    rowsDown = height;
	}
    } else if (midPoint == height) {
	/* Only one span needed */
	if (abszoomy < __glOne) {
	    rowsDown = height;
	} else {
	    rowsUp = height;
	}
    } else {
	/* Almost definitely need two spans to copy this image! */
	intMidPoint = __GL_CEILF(midPoint);

	rowCount = height - intMidPoint;
	if (intMidPoint > rowCount) {
	    rowCount = intMidPoint;
	}

	if (abszoomy > __glOne) {
	    GLint temp;

	    /* Move from outside of image inward */
	    startUp = rowCount - intMidPoint;
	    startDown = rowCount - (height - intMidPoint);
	    rowsUp = intMidPoint;
	    rowsDown = height - rowsUp;

	    if (gc->constants.yInverted) {
		temp = readY - intMidPoint + 1;
	    } else {
		temp = readY + intMidPoint - 1;
	    }

	    if (__GL_FLOORF( (temp - 
		    (rpy-__glHalf-gc->constants.viewportEpsilon)) 
		    / zoomy) == intMidPoint-1) {
		/* 
		** row "intMidPoint-1" copies exactly onto itself.  Let's 
		** make it the midpoint which we converge to.
		*/
		if (startDown) {
		    startDown--;
		} else {
		    startUp++;
		}
	    }
	} else {
	    /* Move from inside of image outward */
	    rowsDown = intMidPoint;
	    rowsUp = height - rowsDown;
	    rpyUp = rpyDown = rpy + zoomy * intMidPoint;
	    if (gc->constants.yInverted) {
		readUp = readY - intMidPoint;
		readDown = readY - intMidPoint + 1;
	    } else {
		readUp = readY + intMidPoint;
		readDown = readY + intMidPoint - 1;
	    }

	    if (__GL_FLOORF( (readDown - 
		    (rpy-__glHalf-gc->constants.viewportEpsilon))
		    / zoomy) == intMidPoint-1) {
		/* 
		** row "intMidPoint-1" copies exactly onto itself.  Let's
		** make it the midpoint which we diverge from.
		*/
		startUp = 1;
	    }
	}
    }

    /* 
    ** Adjust rpyUp and rpyDown so that they will change integer values 
    ** when fragments should be produced.  This basically takes the 0.5
    ** out of the inner loop when these spans are actually iterated.
    */
    if (zoomy > 0) {
	spanInfo->rpyUp = rpyUp + gc->constants.viewportAlmostHalf;
	spanInfo->rpyDown = rpyDown + gc->constants.viewportAlmostHalf - 
		__glOne;
    } else {
	spanInfo->rpyUp = rpyUp + gc->constants.viewportAlmostHalf - __glOne;
	spanInfo->rpyDown = rpyDown + gc->constants.viewportAlmostHalf;
    }
    spanInfo->startUp = startUp;
    spanInfo->startDown = startDown;
    spanInfo->rowsUp = rowsUp;
    spanInfo->rowsDown = rowsDown;
    spanInfo->readUp = readUp;
    spanInfo->readDown = readDown;

    return GL_TRUE;
}

void __glInitCopyPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			    GLint x, GLint y, GLint width, GLint height, 
			    GLenum format)
{
    __GLfloat rpx, rpy;
    __GLfloat zoomx, zoomy;

    rpx = gc->state.current.rasterPos.window.x;
    rpy = gc->state.current.rasterPos.window.y;

#if defined(__GL_HALF_PIXEL_OFFSET)
    rpx -= __glHalf;
    rpy -= __glHalf;
#endif

    spanInfo->x = rpx;
    spanInfo->y = rpy;
    spanInfo->fragz = gc->state.current.rasterPos.window.z *
						gc->constants.depthRescale;
    zoomx = gc->state.pixel.transferMode.zoomX;
    if (zoomx > __glZero) {
	if (zoomx < __glOne) {
	    spanInfo->rendZoomx = __glOne;
	} else {
	    spanInfo->rendZoomx = zoomx;
	}
	spanInfo->coladd = 1;
    } else {
	if (zoomx > (GLfloat) -1.0) {
	    spanInfo->rendZoomx = (GLfloat) -1.0;
	} else {
	    spanInfo->rendZoomx = zoomx;
	}
	spanInfo->coladd = -1;
    }
    spanInfo->zoomx = zoomx;
    zoomy = gc->state.pixel.transferMode.zoomY;
    if (gc->constants.yInverted) {
	zoomy = -zoomy;
    } else {
	spanInfo->y += gc->constants.viewportEpsilon;
    }
    if (zoomy > __glZero) {
	spanInfo->rowadd = 1;
    } else {
	spanInfo->rowadd = -1;
    }
    spanInfo->zoomy = zoomy;
    spanInfo->readX = x + gc->constants.viewportXAdjust;
    if (gc->constants.yInverted) {
	spanInfo->readY = (gc->constants.height - y - 1) + 
		gc->constants.viewportYAdjust;
    } else {
	spanInfo->readY = y + gc->constants.viewportYAdjust;
    }
    spanInfo->dstFormat = spanInfo->srcFormat = format;
    spanInfo->width = width;
    spanInfo->height = height;
    /*
    ** These aren't needed for CopyPixels, but they are read by the
    ** generic pixel path setup code, so something needs to be set
    ** in them.
    */
    spanInfo->srcType = GL_FLOAT;
    spanInfo->srcElementSize = 4;
    spanInfo->srcSwapBytes = GL_FALSE;
    spanInfo->srcImage = NULL;
    spanInfo->srcPackedData = GL_FALSE;
    spanInfo->srcFormat = gc->modes.rgbMode ? GL_RGBA : GL_COLOR_INDEX;
    spanInfo->dstType = GL_FLOAT;
    spanInfo->dstElementSize = 4;
    spanInfo->dstSwapBytes = GL_FALSE;
    spanInfo->dstImage = NULL;
    spanInfo->zeroFillAlpha = GL_FALSE;
    spanInfo->applyClamp = GL_TRUE;
    spanInfo->applyFbScale = GL_TRUE;
    spanInfo->nonColorComp = GL_FALSE;
    spanInfo->dim = 2;
}


/* 
** A CopyPixels with N span modifiers.
*/
void __glCopyPixelSpans(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLfloat newy;
    __GLfloat zoomy;
    GLint inty, i, ySign;
    GLint height;
    GLint numSpanMods = spanInfo->numSpanMods;
    GLint spanModNum;
    void (**spanModifier)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    void (*render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		   GLvoid *inspan);
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
    GLubyte *tmpSpanData, *spanDataIn, *spanDataOut;
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    if (spanInfo->overlap) {
	__glCopyPixelsOverlapping(gc, spanInfo, numSpanMods);
	return;
    }

    reader = spanInfo->spanReader;
    render = spanInfo->spanRender;

    ySign = gc->constants.ySign;
    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;
    spanModifier = spanInfo->spanModifier;

    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->readY += ySign;
	    spanInfo->y = newy;
	    newy += zoomy;
	    i++;
	    assert (i != height);
	}
	inty = (GLint) newy;

	(*reader)(gc, spanInfo, spanData1);
	spanDataIn = spanData1; spanDataOut = spanData2;
	for (spanModNum = 0; spanModNum < numSpanMods; spanModNum++) {
	    (*spanModifier[spanModNum])(gc, spanInfo, spanDataIn, spanDataOut);
	    tmpSpanData = spanDataIn;
	    spanDataIn = spanDataOut;
	    spanDataOut = tmpSpanData;
	}
	(*render)(gc, spanInfo, spanDataIn);

	spanInfo->readY += ySign;
    }
}


/* 
** A CopyPixels with two span modifiers.
*/
void __glCopyPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLfloat newy;
    __GLfloat zoomy;
    GLint inty, i, ySign;
    GLint height;
    void (*reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		   GLvoid *inspan);
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    if (spanInfo->overlap) {
	__glCopyPixelsOverlapping(gc, spanInfo, 2);
	return;
    }

    reader = spanInfo->spanReader;
    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    render = spanInfo->spanRender;

    ySign = gc->constants.ySign;
    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;

    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->readY += ySign;
	    spanInfo->y = newy;
	    newy += zoomy;
	    i++;
	    assert (i != height);
	}
	inty = (GLint) newy;
	(*reader)(gc, spanInfo, spanData1);
	(*span1)(gc, spanInfo, spanData1, spanData2);
	(*span2)(gc, spanInfo, spanData2, spanData1);
	(*render)(gc, spanInfo, spanData1);
	spanInfo->readY += ySign;
    }
}

/* 
** A CopyPixels with one span modifier.
*/
void __glCopyPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLfloat newy;
    __GLfloat zoomy;
    GLint inty, i, ySign;
    GLint height;
    void (*reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		   GLvoid *inspan);
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    if (spanInfo->overlap) {
	__glCopyPixelsOverlapping(gc, spanInfo, 1);
	return;
    }

    reader = spanInfo->spanReader;
    span1 = spanInfo->spanModifier[0];
    render = spanInfo->spanRender;

    ySign = gc->constants.ySign;
    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;

    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->readY += ySign;
	    spanInfo->y = newy;
	    newy += zoomy;
	    i++;
	    assert (i != height);
	}
	inty = (GLint) newy;
	(*reader)(gc, spanInfo, spanData1);
	(*span1)(gc, spanInfo, spanData1, spanData2);
	(*render)(gc, spanInfo, spanData2);
	spanInfo->readY += ySign;
    }
}

/* 
** Copy pixels with no span modifiers.
*/
void __glCopyPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLfloat newy;
    __GLfloat zoomy;
    GLint inty, i, ySign;
    GLint height;
    void (*reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    void (*render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		   GLvoid *inspan);
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    if (spanInfo->overlap) {
	__glCopyPixelsOverlapping(gc, spanInfo, 0);
	return;
    }

    reader = spanInfo->spanReader;
    render = spanInfo->spanRender;

    ySign = gc->constants.ySign;
    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;

    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->readY += ySign;
	    spanInfo->y = newy;
	    newy += zoomy;
	    i++;
	    assert (i != height);
	}
	inty = (GLint) newy;
	(*reader)(gc, spanInfo, spanData1);
	(*render)(gc, spanInfo, spanData1);
	spanInfo->readY += ySign;
    }
}

/*
** Yick!  
**
** This routine is provided to perform CopyPixels when the source and
** destination images overlap.  
**
** It is not designed to go particularly fast, but then overlapping
** copies is probably not too common, and this routine is not typically a 
** large part of the execution overhead anyway.
**
** For more information on copying an image which overlaps its destination,
** check out the hairy comment within the __glClipCopyPixels function.
*/
void __glCopyPixelsOverlapping(__GLcontext *gc, 
			       __GLpixelSpanInfo *spanInfo, GLint modifiers)
{
    GLint i;
    __GLfloat zoomy, newy;
    GLint inty, ySign;
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
    GLubyte spanData3[__GL_MAX_SPAN_SIZE];
    GLubyte *outSpan1, *outSpan2;
    GLint rowsUp, rowsDown;
    GLint startUp, startDown;
    __GLfloat rpyUp, rpyDown;
    GLint readUp, readDown;
    GLint gotUp, gotDown;
    __GLpixelSpanInfo downSpanInfo;
    GLint clipLow, clipHigh;
    GLint startRow, endRow;
    void (*reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    void (*render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		   GLvoid *inspan);

    reader = spanInfo->spanReader;
    render = spanInfo->spanRender;

    if (modifiers & 1) {
	outSpan1 = outSpan2 = spanData3;
    } else {
	outSpan1 = spanData1;
	outSpan2 = spanData2;
    }

    zoomy = spanInfo->zoomy;
    rowsUp = spanInfo->rowsUp;
    rowsDown = spanInfo->rowsDown;
    startUp = spanInfo->startUp;
    startDown = spanInfo->startDown;
    rpyUp = spanInfo->rpyUp;
    rpyDown = spanInfo->rpyDown;
    readUp = spanInfo->readUp;
    readDown = spanInfo->readDown;
    downSpanInfo = *spanInfo;
    downSpanInfo.rowadd = -spanInfo->rowadd;
    downSpanInfo.zoomy = -zoomy;
    spanInfo->y = rpyUp;
    downSpanInfo.y = rpyDown;
    spanInfo->readY = readUp;
    downSpanInfo.readY = readDown;
    gotUp = gotDown = 0;
    ySign = gc->constants.ySign;

    /* Clip upgoing and downgoing spans */
    if (zoomy > 0) {
	clipLow = spanInfo->startRow;
	clipHigh = spanInfo->startRow + spanInfo->rows - 1;

	/* Clip down span first */
	startRow = (GLint) rpyDown;
	endRow = (GLint) (rpyDown - zoomy*rowsDown) + 1;
	if (startRow > clipHigh) startRow = clipHigh;
	if (endRow < clipLow) endRow = clipLow;
	downSpanInfo.startRow = startRow;
	downSpanInfo.rows = startRow - endRow + 1;

	/* Now clip up span */
	startRow = (GLint) rpyUp;
	endRow = (GLint) (rpyUp + zoomy*rowsUp) - 1;
	if (startRow < clipLow) startRow = clipLow;
	if (endRow > clipHigh) endRow = clipHigh;
	spanInfo->startRow = startRow;
	spanInfo->rows = endRow - startRow + 1;
    } else /* zoomy < 0 */ {
	clipHigh = spanInfo->startRow;
	clipLow = spanInfo->startRow - spanInfo->rows + 1;

	/* Clip down span first */
	startRow = (GLint) rpyDown;
	endRow = (GLint) (rpyDown - zoomy*rowsDown) - 1;
	if (startRow < clipLow) startRow = clipLow;
	if (endRow > clipHigh) endRow = clipHigh;
	downSpanInfo.startRow = startRow;
	downSpanInfo.rows = endRow - startRow + 1;

	/* Now clip up span */
	startRow = (GLint) rpyUp;
	endRow = (GLint) (rpyUp + zoomy*rowsUp) + 1;
	if (startRow > clipHigh) startRow = clipHigh;
	if (endRow < clipLow) endRow = clipLow;
	spanInfo->startRow = startRow;
	spanInfo->rows = startRow - endRow + 1;
    }

    while (rowsUp && rowsDown) {
	if (startUp) {
	    startUp--;
	} else {
	    gotUp = 1;
	    rowsUp--;
	    spanInfo->y = rpyUp;
	    newy = rpyUp + zoomy;
	    inty = (GLint) rpyUp;
	    while (rowsUp && (GLint) newy == inty) {
		spanInfo->y = newy;
		newy += zoomy;
		rowsUp--;
		spanInfo->readY += ySign;
	    }
	    if (inty == (GLint) newy) break;
	    rpyUp = newy;
	    (*reader)(gc, spanInfo, spanData1);
	    spanInfo->readY += ySign;
	}
	if (startDown) {
	    startDown--;
	} else {
	    gotDown = 1;
	    rowsDown--;
	    downSpanInfo.y = rpyDown;
	    newy = rpyDown - zoomy;
	    inty = (GLint) rpyDown;
	    while (rowsDown && (GLint) newy == inty) {
		downSpanInfo.y = newy;
		newy -= zoomy;
		rowsDown--;
		downSpanInfo.readY -= ySign;
	    }
	    if (inty == (GLint) newy) {
		if (gotUp) {
		    for (i=0; i<modifiers; i++) {
			if (i & 1) {
			    (*(spanInfo->spanModifier[i]))(gc, spanInfo, 
				    spanData3, spanData1);
			} else {
			    (*(spanInfo->spanModifier[i]))(gc, spanInfo, 
				    spanData1, spanData3);
			}
		    }
		    (*render)(gc, spanInfo, outSpan1);
		}
		break;
	    }
	    rpyDown = newy;
	    (*reader)(gc, &downSpanInfo, spanData2);
	    downSpanInfo.readY -= ySign;
	}

	if (gotUp) {
	    for (i=0; i<modifiers; i++) {
		if (i & 1) {
		    (*(spanInfo->spanModifier[i]))(gc, spanInfo, 
			    spanData3, spanData1);
		} else {
		    (*(spanInfo->spanModifier[i]))(gc, spanInfo, 
			    spanData1, spanData3);
		}
	    }
	    (*render)(gc, spanInfo, outSpan1);
	}

	if (gotDown) {
	    for (i=0; i<modifiers; i++) {
		if (i & 1) {
		    (*(spanInfo->spanModifier[i]))(gc, &downSpanInfo, 
			    spanData3, spanData2);
		} else {
		    (*(spanInfo->spanModifier[i]))(gc, &downSpanInfo, 
			    spanData2, spanData3);
		}
	    }
	    (*render)(gc, &downSpanInfo, outSpan2);
	}
    }

    /*
    ** Only one of the spanners is left to iterate.
    */

    while (rowsUp) {
	/* Do what is left of up spans */
	rowsUp--;
	spanInfo->y = rpyUp;
	newy = rpyUp + zoomy;
	inty = (GLint) rpyUp;
	while (rowsUp && (GLint) newy == inty) {
	    spanInfo->y = newy;
	    newy += zoomy;
	    rowsUp--;
	    spanInfo->readY += ySign;
	}
	if (inty == (GLint) newy) break;
	rpyUp = newy;

	(*reader)(gc, spanInfo, spanData1);
	for (i=0; i<modifiers; i++) {
	    if (i & 1) {
		(*(spanInfo->spanModifier[i]))(gc, spanInfo, 
			spanData3, spanData1);
	    } else {
		(*(spanInfo->spanModifier[i]))(gc, spanInfo, 
			spanData1, spanData3);
	    }
	}
	(*render)(gc, spanInfo, outSpan1);

	spanInfo->readY += ySign;
    }

    while (rowsDown) {
	/* Do what is left of down spans */
	rowsDown--;
	downSpanInfo.y = rpyDown;
	newy = rpyDown - zoomy;
	inty = (GLint) rpyDown;
	while (rowsDown && (GLint) newy == inty) {
	    downSpanInfo.y = newy;
	    newy -= zoomy;
	    rowsDown--;
	    downSpanInfo.readY -= ySign;
	}
	if (inty == (GLint) newy) break;
	rpyDown = newy;

	(*reader)(gc, &downSpanInfo, spanData2);
	for (i=0; i<modifiers; i++) {
	    if (i & 1) {
		(*(spanInfo->spanModifier[i]))(gc, &downSpanInfo, 
			spanData3, spanData2);
	    } else {
		(*(spanInfo->spanModifier[i]))(gc, &downSpanInfo, 
			spanData2, spanData3);
	    }
	}
	(*render)(gc, &downSpanInfo, outSpan2);

	downSpanInfo.readY -= ySign;
    }
}

/*
** Generic implementation of a CopyPixels picker.  Any machine specific
** implementation should provide their own.
*/
void __glSlowPickCopyPixels(__GLcontext *gc, GLint x, GLint y, GLint width,
		            GLint height, GLenum type)
{
    __GLpixelSpanInfo spanInfo;

    __glInitCopyPixelsInfo(gc, &spanInfo, x, y, width, height, type);
    if (!__glClipCopyPixels(gc, &spanInfo)) return;

    __GL_LOCK_PXL_COPY_BUFFERS(gc, type);

    __glGenericPickCopyPixels(gc, &spanInfo);

    __GL_UNLOCK_PXL_COPY_BUFFERS(gc, type);
}

/*
** Generic picker for CopyPixels.  This should be called if no machine
** specific path is provided for this specific version of CopyPixels.
*/
void __glGenericPickCopyPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    void (*cpfn)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
    __GLpixelSpanModInfo spanModInfo;

    spanModInfo.srcType = __GL_SRCDST_FB;
    spanModInfo.dstType = __GL_SRCDST_FB;
    spanModInfo.pixelPath = __GL_PIXPATH_COPYPIX;
    spanModInfo.applyPixelTransfer = GL_TRUE;

    spanInfo->numSpanMods = 0;
    PickSpanModifiers(gc, spanInfo, &spanModInfo);

    spanInfo->spanReader = spanModInfo.reader;
    spanInfo->spanRender = spanModInfo.render;

    switch(spanInfo->numSpanMods) {
    case 0:
	cpfn = __glCopyPixels0;
	break;
    case 1:
	cpfn = __glCopyPixels1;
	break;
    case 2:
	cpfn = __glCopyPixels2;
	break;
    default:
	cpfn = __glCopyPixelSpans;
	break;
    }

    (*cpfn)(gc, spanInfo);
}

/*
** A simple image copying routine with one span modifier.
*/
void __glCopyImage1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);

    height = spanInfo->height;
    span1  = spanInfo->spanModifier[0];
    for (i=0; i<height; i++) {
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanInfo->dstCurrent);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	spanInfo->dstCurrent = (GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement;
    }
}

/*
** A simple image copying routine with two span modifiers.
*/
void __glCopyImage2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i;
    GLint height;
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);

    height = spanInfo->height;
    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    for (i=0; i<height; i++) {
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	(*span2)(gc, spanInfo, spanData1, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement;
    }
}

/*
** A simple image copying routine for N span modifiers.
*/
void __glCopyImageSpans(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i;
    GLint height;
    GLint numSpanMods = spanInfo->numSpanMods;
    GLint spanModNum;
    void (**spanModifier)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
    GLubyte spanData2[__GL_MAX_SPAN_SIZE];
    GLubyte *tmpSpanData, *spanDataIn, *spanDataOut;

    height = spanInfo->height;
    spanModifier = spanInfo->spanModifier;
    for (i=0; i<height; i++) {

	(*spanModifier[0])(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	spanDataIn = spanData1; spanDataOut = spanData2;
	for (spanModNum = 1; spanModNum < numSpanMods-1; spanModNum++) {
	    (*spanModifier[spanModNum])(gc, spanInfo, spanDataIn, spanDataOut);
	    tmpSpanData = spanDataIn;
	    spanDataIn = spanDataOut;
	    spanDataOut = tmpSpanData;
	}
	(*spanModifier[spanModNum])(gc, spanInfo, spanDataIn, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement;
    }
}



/*
** Internal image processing routine.  Used by GetTexImage to transfer from
** internal texture image to the user.  Used by TexImage[12]D to transfer
** from the user to internal texture.  Used for display list optimization of
** textures and DrawPixels.
**
** This routine also supports the pixel format mode __GL_RED_ALPHA which is
** basically a 2 component texture.
**
** If applyPixelTransfer is set to GL_TRUE, pixel transfer modes will be 
** applied as necessary.
*/
void __glGenericPickCopyImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			      GLboolean applyPixelTransfer)
{
    __GLpixelSpanModInfo spanModInfo;
    void (*cpfn)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);

    spanModInfo.srcType = __GL_SRCDST_MEM;
    spanModInfo.dstType = __GL_SRCDST_MEM;
    spanModInfo.pixelPath = __GL_PIXPATH_COPYIMAGE;
    spanModInfo.applyPixelTransfer = applyPixelTransfer;

    spanInfo->numSpanMods = 0;
    PickSpanModifiers(gc, spanInfo, &spanModInfo);


    switch(spanInfo->numSpanMods) {
    case 0:
        /*
        ** Sanity check:  If we have zero span routines, then this simply
        **   isn't going to work.  We need to at least copy the data.
        */
	spanInfo->spanModifier[spanInfo->numSpanMods++] = __glSpanCopy;
	cpfn = __glCopyImage1;
	break;
    case 1:
	cpfn = __glCopyImage1;
	break;
    case 2:
	cpfn = __glCopyImage2;
	break;
    default:
	cpfn = __glCopyImageSpans;
	break;
    }
    (*cpfn)(gc, spanInfo);
}




void __glGenericPickStoreTexture(__GLcontext *gc, 
                                 struct __GLtextureRec *tex, __GLpixelSpanInfo *spanInfo,
                                 GLboolean applyPixelTransfer)
{
    __GLpixelSpanModInfo spanModInfo;
    void (*cpfn)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);

    spanModInfo.srcType = __GL_SRCDST_MEM;
    spanModInfo.dstType = __GL_SRCDST_MEM;
    spanModInfo.pixelPath = __GL_PIXPATH_COPYIMAGE;
    spanModInfo.applyPixelTransfer = applyPixelTransfer;

    spanInfo->numSpanMods = 0;
    PickSpanModifiers(gc, spanInfo, &spanModInfo);
    if (tex->format->store) {
	spanInfo->spanModifier[spanInfo->numSpanMods++] = tex->format->store;
    }

    switch(spanInfo->numSpanMods) {
    case 0:
	spanInfo->spanModifier[spanInfo->numSpanMods++] = __glSpanCopy;
	cpfn = __glCopyImage1;
	break;
    case 1:
	cpfn = __glCopyImage1;
	break;
    case 2:
	cpfn = __glCopyImage2;
	break;
    default:
	cpfn = __glCopyImageSpans;
	break;
    }

    (*cpfn)(gc, spanInfo);
}



void __glGenericPickGetTexture(__GLcontext *gc, __GLtexture *tex, __GLpixelSpanInfo *spanInfo,
                              GLboolean applyPixelTransfer)
{
    __GLpixelSpanModInfo spanModInfo;
    void (*cpfn)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);

    spanModInfo.srcType = __GL_SRCDST_MEM;
    spanModInfo.dstType = __GL_SRCDST_MEM;
    spanModInfo.pixelPath = __GL_PIXPATH_COPYIMAGE;
    spanModInfo.applyPixelTransfer = applyPixelTransfer;

    spanInfo->numSpanMods = 0;
    if (tex->format->fetch) {
	spanInfo->spanModifier[spanInfo->numSpanMods++] = tex->format->fetch;
    }
    PickSpanModifiers(gc, spanInfo, &spanModInfo);

    switch(spanInfo->numSpanMods) {
    case 0:
	spanInfo->spanModifier[spanInfo->numSpanMods++] = __glSpanCopy;
	cpfn = __glCopyImage1;
	break;
    case 1:
	cpfn = __glCopyImage1;
	break;
    case 2:
	cpfn = __glCopyImage2;
	break;
    default:
	cpfn = __glCopyImageSpans;
	break;
    }

    (*cpfn)(gc, spanInfo);
}




/*
** Initializes the src info for ReadImage.  Src info initialization
** is separate from dest info initialization to allow for
** different types of destinations.
*/
void __glInitReadImageSrcInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
				GLint x,
				GLint y,
				GLsizei width,
				GLsizei height)
{
    spanInfo->readX = x + gc->constants.viewportXAdjust;
    if (gc->constants.yInverted) {
	spanInfo->readY = (gc->constants.height - y - 1) + 
		gc->constants.viewportYAdjust;
    } else {
	spanInfo->readY = y + gc->constants.viewportYAdjust;
    }
    spanInfo->srcFormat = gc->modes.rgbMode ? GL_RGBA : GL_COLOR_INDEX;
    spanInfo->srcType = GL_FLOAT;
    spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->zoomx = __glOne;
    spanInfo->x = __glZero;

    spanInfo->srcAlignment = 1;
    spanInfo->srcSkipPixels = 0;
    spanInfo->srcSkipLines = 0;
    spanInfo->srcLsbFirst = GL_FALSE;
    spanInfo->srcSwapBytes = GL_FALSE;
    spanInfo->srcLineLength = spanInfo->width;
    /*
    ** These aren't needed for ReadImage, but they are read by the
    ** generic pixel path setup code, so something needs to be set
    ** in them.
    */
    spanInfo->srcType = GL_FLOAT;
    spanInfo->srcElementSize = 4;
    spanInfo->srcSwapBytes = GL_FALSE;
    spanInfo->srcImage = NULL;
    spanInfo->srcPackedData = GL_FALSE;
}

/*
** Generic picker for reading from the frame buffer into an internal
** buffer.  This should be called if no machine specific path is provided 
** for this specific version.  This is used by CopyTexImage.
*/
void __glGenericPickReadImage(__GLcontext *gc, __GLtexture *tex, __GLpixelSpanInfo *spanInfo)
{
    void (*rpfn)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
    __GLpixelSpanModInfo spanModInfo;

    spanModInfo.srcType = __GL_SRCDST_FB;
    spanModInfo.dstType = __GL_SRCDST_MEM;
    spanModInfo.pixelPath = __GL_PIXPATH_READIMAGE;
    spanModInfo.applyPixelTransfer = GL_TRUE;

    spanInfo->numSpanMods = 0;
    PickSpanModifiers(gc, spanInfo, &spanModInfo);
    if (tex->format->store) {
	spanInfo->spanModifier[spanInfo->numSpanMods++] = tex->format->store;
    }
    spanInfo->spanReader = spanModInfo.reader;

    /*
    ** Pick a ReadPixels routine that uses the right number of span 
    ** modifiers.
    */

    switch(spanInfo->numSpanMods) {
      case 0:
	rpfn = __glReadPixels0;
	break;
      case 1:
	rpfn = __glReadPixels1;
	break;
      case 2:
	rpfn = __glReadPixels2;
	break;
      default:
	rpfn = __glReadPixelSpans;
	break;
    }

    (*rpfn)(gc, spanInfo);
}


/*
** Used to set up writing into mem objects without doing any packing.
*/
/* ARGSUSED */
void __glInitMemLoad(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			GLenum internalFormat, const GLvoid *buf)
{
    spanInfo->rscale = 1.0;
    spanInfo->gscale = 1.0;
    spanInfo->bscale = 1.0;
    spanInfo->ascale = 1.0;
    spanInfo->rbias = 0.0;
    spanInfo->gbias = 0.0;
    spanInfo->bbias = 0.0;
    spanInfo->abias = 0.0;

    spanInfo->dstImage = buf;
    spanInfo->dstSkipPixels = 0;
    spanInfo->dstSkipLines = 0;
    spanInfo->dstSwapBytes = GL_FALSE;
    spanInfo->dstLsbFirst = GL_TRUE;
    spanInfo->dstLineLength = spanInfo->width;
    spanInfo->dim = 0; /* don't convolve */

    switch(internalFormat) {
      case GL_LUMINANCE:
	spanInfo->dstFormat = GL_RED;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_LUMINANCE_ALPHA:
	spanInfo->dstFormat = __GL_RED_ALPHA;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_RGB:
	spanInfo->dstFormat = GL_RGB;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_RGBA:
	spanInfo->dstFormat = GL_RGBA;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_ALPHA:
	spanInfo->dstFormat = GL_ALPHA;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_INTENSITY:
	spanInfo->dstFormat = GL_RED;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      default:
	assert(0);
	break;
    }
}

/*
** Used to set up getting data from mem objects without doing any unpacking.
*/
/*ARGSUSED*/
void __glInitMemGet(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			GLsizei width, GLsizei height, GLenum internalFormat,
			const GLvoid *buf)
{
    spanInfo->srcImage = buf;
    spanInfo->srcSkipPixels = 0;
    spanInfo->srcSkipLines = 0;
    spanInfo->srcSwapBytes = GL_FALSE;
    spanInfo->srcLsbFirst = GL_TRUE;
    spanInfo->srcLineLength = width;
    spanInfo->dim = 0; /* don't convolve */

    switch(internalFormat) {
      case GL_LUMINANCE:
	spanInfo->srcFormat = GL_RED;
	spanInfo->srcType = GL_FLOAT;
	spanInfo->srcAlignment = 4;
	break;
      case GL_LUMINANCE_ALPHA:
	spanInfo->srcFormat = __GL_RED_ALPHA;
	spanInfo->srcType = GL_FLOAT;
	spanInfo->srcAlignment = 4;
	break;
      case GL_RGB:
	spanInfo->srcFormat = GL_RGB;
	spanInfo->srcType = GL_FLOAT;
	spanInfo->srcAlignment = 4;
	break;
      case GL_RGBA:
	spanInfo->srcFormat = GL_RGBA;
	spanInfo->srcType = GL_FLOAT;
	spanInfo->srcAlignment = 4;
	break;
      case GL_ALPHA:
	spanInfo->srcFormat = GL_ALPHA;
	spanInfo->srcType = GL_FLOAT;
	spanInfo->srcAlignment = 4;
	break;
      case GL_INTENSITY:
	spanInfo->srcFormat = GL_RED;
	spanInfo->srcType = GL_FLOAT;
	spanInfo->srcAlignment = 4;
	break;
      default:
	assert(0);
	break;
    }
}

/*
** Used to set up the host memory source when loading a mem object.
** "dltex" is GL_TRUE if the mem object is a display-listed texture.
*/
void __glInitMemUnpack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			   GLsizei width, GLsizei height, GLsizei depth,
			   GLenum format, GLenum type, const GLvoid *buf,
			   GLboolean dltex)
{
    spanInfo->x = 0;
    spanInfo->zoomx = __glOne;
    spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->depth = depth;
    spanInfo->srcFormat = format;
    spanInfo->srcType = type;
    spanInfo->srcImage = buf;

    /* Set the remaining source modes according to the pixel state */
    __glLoadUnpackModes(gc, spanInfo, dltex);
}

/*
** Used to set up the host memory destination when getting a mem object.
*/
void __glInitMemPack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			 GLsizei width, GLsizei height, GLsizei depth,
			 GLenum format, GLenum type, const GLvoid *buf)
{
    spanInfo->x = 0;
    spanInfo->zoomx = __glOne;
    spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->depth = depth;
    spanInfo->dstFormat = format;
    spanInfo->dstType = type;
    spanInfo->dstImage = buf;

    /* Set the remaining source modes according to the pixel state */
    __glLoadPackModes(gc, spanInfo);
}


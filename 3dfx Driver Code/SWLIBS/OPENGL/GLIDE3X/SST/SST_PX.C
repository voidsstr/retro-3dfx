#include <windows.h>
#include <math.h>
#include "context.h"
#include "global.h"
#include "pixel.h"
#include "imports.h"
#include <glide.h>
#include "sst_globals.h"

void __glSSTReadPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i, ySign;
    GLint height;
    void (*reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);

    reader = spanInfo->spanReader;

    ySign = gc->constants.ySign;
    height = spanInfo->height;

    __GL_LOCK_BUFFERS(gc);

    for (i=0; i<height; i++) {
	(*reader)(gc, spanInfo, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLvoid *) ((GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement);
	spanInfo->readY += ySign;
    }
    __GL_UNLOCK_BUFFERS(gc);
}

static
void readRGB(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, GLvoid *span)
{
    GLint i;
    GLint width;
    GLboolean hasAlpha;
    unsigned char *spanData;
    __GLcolorBuffer *cfb;
    int x, y, stride;
    unsigned char *p;
    unsigned short value;

    width = spanInfo->width;
    spanData = (unsigned char *) span;

    x = spanInfo->readX - gc->constants.viewportXAdjust;
    y = spanInfo->readY - gc->constants.viewportYAdjust;

    cfb = gc->readBuffer;
    stride = cfb->buf.byteWidth;
    p = cfb->buf.base;
    p += y * stride;
    p += x * 2;

    hasAlpha = spanInfo->dstFormat == GL_RGBA;

    if (hasAlpha) {
	for (i=0; i<width; i++) {
	    value = *(unsigned short *)p;
	    p += 2;

	    spanData[0] = ((value & 0xf800) >> 8) | (value >> 13);
	    spanData[1] = ((value & 0x07e0) >> 3) | ((value & 0x0600) >> 9);
	    spanData[2] = ((value & 0x001f) << 3) | ((value & 0x001c) >> 2);
	    spanData[3] = 0xff;
	    spanData += 4;
	}
    } else {
	for (i=0; i<width; i++) {
	    value = *(unsigned short *)p;
	    p += 2;

	    spanData[0] = ((value & 0xf800) >> 8) | (value >> 13);
	    spanData[1] = ((value & 0x07e0) >> 3) | ((value & 0x0600) >> 9);
	    spanData[2] = ((value & 0x001f) << 3) | ((value & 0x001c) >> 2);
	    spanData += 3;
	}
    }    
}

/*
** Generic picker for ReadPixels.  This should be called if no machine
** specific path is provided for this specific version of ReadPixels.
*/
void __glSSTPickReadPixels(__GLcontext *gc, GLint x, GLint y,
			   GLsizei width, GLsizei height,
			   GLenum format, GLenum type, const GLvoid *pixels)
{
    void (*reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    void (*rpfn)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
    GLint spanCount;
    __GLpixelSpanModInfo spanModInfo;
    __GLpixelMachine *pm;
    __GLpixelSpanInfo spanInfo;

    __glInitReadPixelsInfo(gc, &spanInfo, x, y, width, height, format, 
			   type, pixels);

    if (!__glClipReadPixels(gc, &spanInfo)) return;

    __glInitPacker(gc, &spanInfo);

    spanModInfo.srcType = __GL_SRCDST_FB;
    spanModInfo.dstType = __GL_SRCDST_MEM;
    spanModInfo.pixelPath = __GL_PIXPATH_READPIX;
    spanModInfo.applyPixelTransfer = GL_TRUE;

    pm = &gc->pixel;
    if (spanInfo.dstSwapBytes == GL_FALSE &&
	(spanInfo.dstFormat == GL_RGB || spanInfo.dstFormat == GL_RGBA) &&
	spanInfo.dstType == GL_UNSIGNED_BYTE &&
	!pm->modifyRGBA) {

	spanInfo.numSpanMods = 0;
	spanInfo.spanReader = readRGB;
	__glSSTReadPixels0(gc, &spanInfo);
	return;
    } else {
	spanCount = __glPickSpanModifiers(gc, &spanInfo, &spanModInfo);
    }

    reader = spanModInfo.reader;


    /*
    ** Pick a ReadPixels routine that uses the right number of span 
    ** modifiers.
    */

    spanInfo.spanReader = reader;

    switch(spanCount) {
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
    spanInfo.numSpanMods = spanCount;

    (*rpfn)(gc, &spanInfo);
}




void __glSSTDrawPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i, ySign;
    GLint height;
    void (*render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);

    render = spanInfo->spanRender;

    ySign = gc->constants.ySign;
    height = spanInfo->height;

    __GL_LOCK_BUFFERS(gc);

    for (i=0; i<height; i++) {
	(*render)(gc, spanInfo, spanInfo->srcCurrent);
	spanInfo->srcCurrent = (GLvoid *) ((GLubyte *) spanInfo->srcCurrent +
		spanInfo->srcRowIncrement);
	spanInfo->y += ySign;
    }
    __GL_UNLOCK_BUFFERS(gc);
}

static
void renderRGB(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, GLvoid *span)
{
    GLint i;
    GLint width;
    unsigned char *spanData;
    __GLcolorBuffer *cfb;
    int x, y, stride, skip;
    unsigned char *p;
    unsigned short *value;

    width = spanInfo->width;
    spanData = (unsigned char *) span;

    x = spanInfo->x - gc->constants.viewportXAdjust;
    y = spanInfo->y - gc->constants.viewportYAdjust;

    cfb = gc->drawBuffer;
    stride = cfb->buf.byteWidth;
    p = cfb->buf.base;
    p += y * stride;
    p += x * 2;

    skip = spanInfo->srcFormat == GL_RGB ? 3 : 4;
    
    for (i=0; i<width; i++) {
	value = (unsigned short *)p;
	p += 2;

	*value = (((spanData[0] & 0xf8) << 8) |
		  ((spanData[1] & 0xfc) << 3) |
		  ((spanData[2] & 0xf8) >> 3));

	spanData += skip;
    }    
}


void __glSSTPickDrawPixels(__GLcontext *gc, GLint width, GLint height,
			   GLenum format, GLenum type, const GLvoid *pixels,
			   GLboolean packed)
{
    void (*dpfn)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
    void (*render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
    GLint spanCount;
    __GLpixelSpanModInfo spanModInfo;
    __GLpixelMachine *pm;
    __GLpixelSpanInfo spanInfo;
    
    __glInitDrawPixelsInfo(gc, &spanInfo, width, height, format, type, pixels);
    __glLoadUnpackModes(gc, &spanInfo, packed);

    if(!__glClipDrawPixels(gc, &spanInfo)) return;

    __glInitUnpacker(gc, &spanInfo);

    spanModInfo.srcType = __GL_SRCDST_MEM;
    spanModInfo.dstType = __GL_SRCDST_FB;
    spanModInfo.pixelPath = __GL_PIXPATH_DRAWPIX;
    spanModInfo.applyPixelTransfer = GL_TRUE;

    pm = &gc->pixel;
    if (spanInfo.srcSwapBytes == GL_FALSE &&
	(spanInfo.srcFormat == GL_RGB || spanInfo.srcFormat == GL_RGBA) &&
	spanInfo.srcType == GL_UNSIGNED_BYTE &&
	!pm->modifyRGBA && pm->fastRGBA) {

	spanInfo.numSpanMods = 0;
	spanInfo.spanRender = renderRGB;
	__glSSTDrawPixels0(gc, &spanInfo);
	return;
    } else {
	spanCount = __glPickSpanModifiers(gc, &spanInfo, &spanModInfo);
    }

    render = spanModInfo.render;

    /*
    ** Pick a DrawPixels function that applies the correct number of 
    ** span modifiers.
    */

    switch(spanCount) {
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

    spanInfo.numSpanMods = spanCount;

    spanInfo.spanRender = render;
    (*dpfn)(gc, &spanInfo);
}


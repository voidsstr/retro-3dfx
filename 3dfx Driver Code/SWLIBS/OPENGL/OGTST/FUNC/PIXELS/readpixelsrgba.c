#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "ogtst.h"

#define ERROR_MULT 3.0         /* Tolerance is This/(2^nbits - 1) */
typedef struct {
    GLint Right, Top;
    GLint dimension;            /* Dimension of write square */
    GLenum dtype;               /* draw type */
    GLenum rtype;               /* read type */
    GLenum dformat;             /* draw and read format */
    GLenum rformat;             /* draw and read format */
    GLfloat *dpixels;           /* drawn pixels */
    GLvoid *rpixels;            /* read pixels */
} PixelInfo, *PixelInfoPtr;

static GLfloat
tolerance(PixelInfoPtr pip, EnvironmentPtr ep)
{
    GLint bits, *visualBits = (GLint *) &ep->rgba_bits[0];

    switch(pip->rformat) {
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
        bits = visualBits[pip->rformat - GL_RED];
        break;
      case GL_RGB:
      case GL_RGBA:
#ifdef GL_ABGR_EXT
      case GL_ABGR_EXT:
#endif
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
        bits = visualBits[0] < visualBits[1] ? 
            (visualBits[0] < visualBits[2] ? visualBits[0] : visualBits[2]) :
            (visualBits[1] < visualBits[2] ? visualBits[1] : visualBits[2]);
        if ((pip->rformat == GL_RGBA ||
#ifdef GL_ABGR_EXT
             pip->rformat == GL_ABGR_EXT ||
#endif
             pip->rformat == GL_LUMINANCE_ALPHA) &&
            bits > visualBits[3] && visualBits[3] != 0)
            bits = visualBits[3];
        break;
      default:
        assert(0);
    }
    if (bits == 0) {
        /* Pick the maximum bits for this visual */
        bits = visualBits[0];
        if (visualBits[1] > bits)
            bits = visualBits[1];
        if (visualBits[2] > bits)
            bits = visualBits[2];
        if (visualBits[3] > bits)
            bits = visualBits[3];
    }
    switch (pip->rtype) {
      case GL_BYTE:
        if (bits > 7)
            return ERROR_MULT/0x7f;
        break;
      case GL_UNSIGNED_BYTE:
        if (bits > 8)
            return ERROR_MULT/0xff;
        break;
    }
    return ERROR_MULT/((1 << bits) - 1);
}

static GLint number_components(GLenum format) 
{
    switch(format) {
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_LUMINANCE:
        return 1;
      case GL_LUMINANCE_ALPHA:
        return 2;
      case GL_RGB:
        return 3;
      case GL_RGBA:
#ifdef GL_ABGR_EXT
      case GL_ABGR_EXT:
#endif
        return 4;
      default:
        ogEnvLog(OG_LINTERNALERROR,"bad format 0x%X in case\n", format);
        return 0;
    }
}

static void
make_pixels(PixelInfoPtr pip, EnvironmentPtr ep)
{
    GLint n_comp;
    GLint i, j, k, l, m;
    GLint levels, compOffset;
    GLfloat *pPix;

    n_comp = number_components(pip->dformat);
    switch(pip->dformat) {
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
        i = ep->rgba_bits[pip->dformat - GL_RED];
        break;
      case GL_RGB:
      case GL_RGBA:
#ifdef GL_ABGR_EXT
      case GL_ABGR_EXT:
#endif
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
        i = ep->rgba_bits[0] < ep->rgba_bits[1] ? 
            (ep->rgba_bits[0] < ep->rgba_bits[2] ?
             ep->rgba_bits[0] : ep->rgba_bits[2]) :
                 (ep->rgba_bits[1] < ep->rgba_bits[2] ?
                  ep->rgba_bits[1] : ep->rgba_bits[2]);
        if((GL_LUMINANCE_ALPHA == pip->dformat || GL_RGBA == pip->dformat) &&
           i > ep->rgba_bits[3] && ep->rgba_bits[3] != 0)
            i = ep->rgba_bits[3];
        break;
      default:
        assert(0);
    }
    if (i == 0)
        i = 8;
    ogEnvLog(OG_LPARAMETERS, "make_pixels(): %s, bits = %d\n",
             ogEnvPixelFormatName(pip->dformat), i);
    levels = 1 << i;
    if (levels > pip->dimension * pip->dimension)
        levels = pip->dimension * pip->dimension;
    compOffset = levels / n_comp;

    for(pPix = pip->dpixels, i = 0; i < pip->dimension; i++)
        for (k= 3; k; k--)
            for(j = 0; j < pip->dimension; j++)
                for (l = 3; l; l--)
                    for(m=0; m < n_comp; m++, pPix++)
                        *pPix = ((i * pip->dimension + j + m * compOffset) %
                                 levels) / (GLfloat) (levels - 1);
}

static GLvoid compare_pixels(PixelInfoPtr pip, EnvironmentPtr ep)
{
    GLint i, j, k, n_rcomp, n_dcomp, readPixStart;
    GLfloat tol;
    GLfloat *pDrawPix;
    GLfloat expected_value, read_value;

    glReadPixels(pip->Right/2, pip->Top/2, 3 * pip->dimension,
                 3 * pip->dimension, pip->rformat, pip->rtype, pip->rpixels);
    ogEnvLog(OG_LPARAMETERS,
             "glReadPixels(%d, %d, %d, %d, %s, %s, pip->rpixels)\n", 
             pip->Right/2, pip->Top/2, 3 * pip->dimension, 3 * pip->dimension,
             ogEnvPixelFormatName(pip->rformat), ogEnvDataTypeName(pip->rtype));

    n_dcomp = number_components(pip->dformat);
    n_rcomp = number_components(pip->rformat);
    tol = tolerance(pip, ep);
#ifdef DEBUG
    ogEnvLog(OG_LINTERNALDEBUG,"n_dcomp = %d, n_rcomp = %d, tol = %f\n",
             n_dcomp, n_rcomp, tol);
#endif

    /* We look at the draw pixel in the lower left corner of each 3x3
     * pixel square, but at center pixel of each of the 3x3 squares that
     * of the read pixels.
     */
    pDrawPix = pip->dpixels;
    readPixStart = (3 * pip->dimension + 1) * n_rcomp;
    n_dcomp *= 3;               /* to save multiplies in the loop */
    for (i = pip->dimension; i; i--, pDrawPix += 2 * pip->dimension * n_dcomp,
             readPixStart += 2 * pip->dimension * 3 * n_rcomp) {
        for (j = pip->dimension ; j;
             j--, pDrawPix += n_dcomp, readPixStart += 3 * n_rcomp) {
	    for(k=0; k < n_rcomp; k++) {
                switch(pip->rformat) {
                  case GL_RED:
                  case GL_GREEN:
                  case GL_BLUE:
                    expected_value = ep->rgba_bits[pip->rformat - GL_RED] ?
                        pDrawPix[pip->rformat - GL_RED] : 0;
		    break;
                  case GL_ALPHA:
                    expected_value = ep->rgba_bits[3] ?
                        pDrawPix[3] : 1;
		    break;
                  case GL_RGB:
                    expected_value = ep->rgba_bits[k] ?
                        pDrawPix[k] : 0;
		    break;
                  case GL_RGBA:
                    expected_value = ep->rgba_bits[k] ?
                        pDrawPix[k] : (k != 3) ? 0 : 1;
		    break;
#ifdef GL_ABGR_EXT
                  case GL_ABGR_EXT:
                    expected_value = ep->rgba_bits[3 - k] ?
                        pDrawPix[3 - k] : (k != 0) ? 0 : 1;
		    break;
#endif
                  case GL_LUMINANCE:
                  case GL_LUMINANCE_ALPHA: 
                    if (k & 0x1) { /* 2nd VALUE IS ALPHA */
                        expected_value = ep->rgba_bits[3] ? 
                            pDrawPix[3] : 1;
                    } else { /* 1st VALUE IS LUMINANCE */
                        expected_value =
                            (ep->rgba_bits[0] ? pDrawPix[0] : 0) +
                            (ep->rgba_bits[1] ? pDrawPix[1] : 0) +
                            (ep->rgba_bits[2] ? pDrawPix[2] : 0);
                        if(expected_value > 1)
                            expected_value = 1;
                    }
		    break;
                  default:
                    ogEnvLog(OG_LINTERNALERROR,"Bad draw format 0x%X in\
 compare_pixels\n", pip->dformat);
		    break;
		}
		switch(pip->rtype) {
                  case GL_UNSIGNED_BYTE:
                    read_value =
                        OGTST_UB_TO_F(
                            ((GLubyte*)pip->rpixels)[readPixStart + k]);
		    break;
                  case GL_BYTE:
                    read_value = OGTST_B_TO_F(
                        ((GLbyte*)pip->rpixels)[readPixStart + k]);
		    break;
                  case GL_UNSIGNED_SHORT:
                    read_value = OGTST_US_TO_F(
                        ((GLushort*)pip->rpixels)[readPixStart + k]);
		    break;
                  case GL_SHORT:
                    read_value = OGTST_S_TO_F(
                        ((GLshort*)pip->rpixels)[readPixStart + k]);
		    break;
                  case GL_UNSIGNED_INT:
                    read_value = OGTST_UI_TO_F(
                        ((GLuint*)pip->rpixels)[readPixStart + k]);
		    break;
                  case GL_INT:
                    read_value = OGTST_I_TO_F(
                        ((GLint*)pip->rpixels)[readPixStart + k]);
		    break;
                  case GL_FLOAT:
                    read_value = ((GLfloat*)pip->rpixels)[readPixStart + k];
		    break;
                  case GL_BITMAP:
                  default:
                    ogEnvLog(OG_LINTERNALERROR,
                             "bad type 0x%X in compare_pixels\n", pip->rtype);
		    break;
		}
		/* ACTUAL COMPARISON */
		if(fabsf(read_value - expected_value) > tol) {
		    ogEnvLog(OG_LFAIL,
                             "%s %s pix(%i,%i)[%i], expected %g, read %g\n",
                             ogEnvPixelFormatName(pip->rformat),
                             ogEnvDataTypeName(pip->rtype),
                             3*(pip->dimension - i) + 1,
                             3*(pip->dimension - j) + 1,
                             k, expected_value, read_value);
		}
#ifdef DEBUG
                else ogEnvLog(OG_LINTERNALDEBUG,
                                "%s %s pix(%i,%i)[%i], expected %g, read %g\n",
                                ogEnvPixelFormatName(pip->rformat),
                                ogEnvDataTypeName(pip->rtype),
                                3*(pip->dimension - i) + 1,
                                3*(pip->dimension - j) + 1,
                                k, expected_value, read_value);
#endif
	    }
	}
    }
}


#if 0
static GLvoid print_pixels(PixelInfoPtr pip)
{
    GLint i,j,k,n_comp;

    n_comp = number_components(pip->dformat);

    for(i=0; i < 3 * pip->dimension; i++) {
	for(j=0; j < 3 * pip->dimension; j++) {
	    for(k=0; k < n_comp; k++) {
	        ogEnvLog(OG_LALWAYS,"[%d] = %g\n",n_comp * (i*pip->dimension + j) + k,
                       pip->dpixels[n_comp * (i*pip->dimension + j) + k]);
	    }
	}
    }
}
#endif

static GLboolean
init(PixelInfoPtr pip, EnvironmentPtr ep)
{
    GLboolean crp_valid;

    pip->dpixels = (GLfloat *)
        ogLibMalloc(3 * pip->dimension * 3 * pip->dimension * 4 * sizeof(GLfloat));
    pip->rpixels = 
        ogLibMalloc(3 * pip->dimension * 3 * pip->dimension * 4 * sizeof(GLfloat));
    if (pip->dpixels == NULL || pip->rpixels == NULL) {
        ogEnvLog (OG_LFAIL,"Out of memory\n");
	return GL_TRUE;
    }

    make_pixels(pip, ep);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, pip->Right, 0, pip->Top, -1, 1);
    ogEnvLog(OG_LPARAMETERS,"glOrtho(0, %d, 0, %d, -1, 1)\n",
					pip->Right, pip->Top);

    glRasterPos3i(pip->Right/2, pip->Top/2, 0);
    ogEnvLog(OG_LPARAMETERS,"glRasterPos3i(%d, %d, 0)\n", 
					pip->Right/2, pip->Top/2);

    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &crp_valid);
    ogEnvLog(OG_LPARAMETERS,
             "glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, %d)\n",
             crp_valid);
    if(!crp_valid) {
	ogEnvLog (OG_LFAIL,"Invalid Raster Pos\n");
	return GL_FALSE;
    }

    glDrawPixels(3 * pip->dimension, 3 * pip->dimension, pip->dformat, pip->dtype,
                 pip->dpixels);
    ogEnvLog(OG_LPARAMETERS,"glDrawPixels(%d,%d,%s,%s,pip->dpixels)\n",
             3 * pip->dimension, 3 * pip->dimension,
             ogEnvPixelFormatName(pip->dformat), ogEnvDataTypeName(pip->dtype));

    return GL_TRUE;
}

TESTMOD (ReadPixelsRGBA)
{
    PixelInfo pi;
    Environment env;
    GLint i, j;
    GLenum formats[] = {
        GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE,
        GL_LUMINANCE_ALPHA,
#ifdef GL_ABGR_EXT        
        GL_ABGR_EXT,
#endif
    };

    glClearColor(1, 1, 1, 1);
    ogEnvLog(OG_LPARAMETERS,"glClearColor(1, 1, 1, 1)\n");
    glClear(GL_COLOR_BUFFER_BIT);
    ogEnvLog(OG_LPARAMETERS,"glClear(GL_COLOR_BUFFER_BIT)\n");
    
    ogLibGetEnvironment(&env);
#ifdef DEBUG
    ogLibShowEnvironment();
#endif

    glDisable(GL_DITHER);
    ogEnvLog(OG_LPARAMETERS,"glDisable(GL_DITHER)\n");
    pi.Right = ogEnvQuery(OG_XWSIZE);
    pi.Top = ogEnvQuery(OG_YWSIZE);

    /* Be sure that this is a multiple of 4 */
    pi.dimension = 8;

    pi.dtype = GL_FLOAT;
    pi.dformat = GL_RGBA;
    if (init(&pi, &env))
        while(pass--) {
    	    ogEnvLog(OG_LPARAMETERS,"pass = %d\n", pass+1);
            for (i = 0; i < sizeof(formats)/sizeof(GLenum); i++){
                for(j = GL_BYTE; j <= GL_FLOAT; j++) {
                    pi.rformat = formats[i];
                    pi.rtype = j;
                    compare_pixels(&pi, &env);
                }
            }
        }
    if (pi.dpixels != NULL)
        ogLibFree(pi.dpixels);
    if (pi.rpixels != NULL)
        ogLibFree(pi.rpixels);
}

CLEANUP (ReadPixelsRGBA)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultClears();
    glEnable(GL_DITHER);
    ogLibSetDefaultRasterPos();
    ogLibSetDefaultBuffers();
}

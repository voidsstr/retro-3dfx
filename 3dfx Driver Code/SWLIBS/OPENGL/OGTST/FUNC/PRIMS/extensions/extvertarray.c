/**************************************************************************
 *                                                                        *
 *               Copyright (C) 1994, Silicon Graphics, Inc.               *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 **************************************************************************/

/* 
 * This test verifies the vertex array extension, including DrawArraysEXT,
 * ArrayElementEXT
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "ogtst.h"

#ifdef GL_DOUBLE_EXT

#define MAX_SIZE 64
#define W 7
#define H 9

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

static PFNGLARRAYELEMENTEXTPROC ArrayElementEXT;
static PFNGLCOLORPOINTEREXTPROC ColorPointerEXT;
static PFNGLDRAWARRAYSEXTPROC DrawArraysEXT;
static PFNGLEDGEFLAGPOINTEREXTPROC EdgeFlagPointerEXT;
static PFNGLGETPOINTERVEXTPROC GetPointervEXT;
static PFNGLINDEXPOINTEREXTPROC IndexPointerEXT;
static PFNGLNORMALPOINTEREXTPROC NormalPointerEXT;
static PFNGLTEXCOORDPOINTEREXTPROC TexCoordPointerEXT;
static PFNGLVERTEXPOINTEREXTPROC VertexPointerEXT;

static const GLenum vtypes[] = {
  GL_SHORT,
  GL_INT, 
  GL_FLOAT, 
  GL_DOUBLE_EXT,
};
static const int vtype_count = 4;
/* why are vfuncs not vertex calls?  because then we can't read the result
 * back.  instead we'll use glRasterPos for type conversion and reading back
 * the results, then look at the pixel in the framebuffer to check the results
 * of the glDrawArray and glArrayElement calls */
static GLvoid (__stdcall*vfuncs[])(const GLvoid *) = {
    (GLvoid (__stdcall*)(const GLvoid *))glRasterPos2sv,
    (GLvoid (__stdcall*)(const GLvoid *))glRasterPos2iv,
    (GLvoid (__stdcall*)(const GLvoid *))glRasterPos2fv,
    (GLvoid (__stdcall*)(const GLvoid *))glRasterPos2dv,
    (GLvoid (__stdcall*)(const GLvoid *))glRasterPos3sv,
    (GLvoid (__stdcall*)(const GLvoid *))glRasterPos3iv,
    (GLvoid (__stdcall*)(const GLvoid *))glRasterPos3fv,
    (GLvoid (__stdcall*)(const GLvoid *))glRasterPos3dv,
    (GLvoid (__stdcall*)(const GLvoid *))glRasterPos4sv,
    (GLvoid (__stdcall*)(const GLvoid *))glRasterPos4iv,
    (GLvoid (__stdcall*)(const GLvoid *))glRasterPos4fv,
    (GLvoid (__stdcall*)(const GLvoid *))glRasterPos4dv,
};

static const GLenum ntypes[] = {
  GL_BYTE,
  GL_SHORT,
  GL_INT,
  GL_FLOAT,
  GL_DOUBLE_EXT,
};
static const int ntype_count = 5;
static GLvoid (__stdcall*nfuncs[])(const GLvoid *) = {
    (GLvoid (__stdcall*)(const GLvoid *))glNormal3bv,
    (GLvoid (__stdcall*)(const GLvoid *))glNormal3sv,
    (GLvoid (__stdcall*)(const GLvoid *))glNormal3iv,
    (GLvoid (__stdcall*)(const GLvoid *))glNormal3fv,
    (GLvoid (__stdcall*)(const GLvoid *))glNormal3dv,
};

static const GLenum ctypes[] = {
  GL_BYTE,
  GL_UNSIGNED_BYTE,
  GL_SHORT,
  GL_UNSIGNED_SHORT,
  GL_INT,
  GL_UNSIGNED_INT,
  GL_FLOAT,
  GL_DOUBLE_EXT,
};
static const int ctype_count = 8;
static GLvoid (__stdcall*cfuncs[])(const GLvoid *) = {
    (GLvoid (__stdcall*)(const GLvoid *))glColor3bv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor3ubv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor3sv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor3usv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor3iv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor3uiv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor3fv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor3dv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor4bv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor4ubv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor4sv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor4usv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor4iv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor4uiv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor4fv,
    (GLvoid (__stdcall*)(const GLvoid *))glColor4dv,
};

static const GLenum itypes[] = {
  GL_SHORT, 
  GL_INT,
  GL_FLOAT,
  GL_DOUBLE_EXT,
};
static const int itype_count = 4;
static GLvoid (__stdcall*ifuncs[])(const GLvoid *) = {
    (GLvoid (__stdcall*)(const GLvoid *))glIndexsv,
    (GLvoid (__stdcall*)(const GLvoid *))glIndexiv,
    (GLvoid (__stdcall*)(const GLvoid *))glIndexfv,
    (GLvoid (__stdcall*)(const GLvoid *))glIndexdv,
};

static const GLenum ttypes[] = {
  GL_SHORT,
  GL_INT,
  GL_FLOAT,
  GL_DOUBLE_EXT,
};
static const int ttype_count = 4;
static GLvoid (__stdcall*tfuncs[])(const GLvoid *) = {
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord1sv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord1iv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord1fv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord1dv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord2sv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord2iv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord2fv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord2dv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord3sv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord3iv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord3fv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord3dv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord4sv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord4iv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord4fv,
    (GLvoid (__stdcall*)(const GLvoid *))glTexCoord4dv,
};

static int enum_size(GLenum x)
{
    switch(x) {
      case GL_BYTE:  case GL_UNSIGNED_BYTE:
	return sizeof(GLbyte);
      case GL_SHORT:  case GL_UNSIGNED_SHORT:
	return sizeof(GLshort);
      case GL_INT:  case GL_UNSIGNED_INT:
	return sizeof(GLint);
      case GL_FLOAT:
	return sizeof(GLfloat);
      case GL_DOUBLE_EXT:
	return sizeof(GLdouble);
      default:
	assert(0);
	return 0;
    }
}

static void draw_read_color(GLuint *c)
{
    glBegin(GL_POLYGON);
    glVertex2f(0, 0);
    glVertex2f(5, 0);
    glVertex2f(5, 5);
    glVertex2f(0, 5);
    glEnd();
    ogLibReadPixels(2, 2, 2, 2, c);
    ogLibPixelCheck(2, 2, *c);
}

#define TEST_ENABLE(a, init_val) \
if ((init_val && (!glIsEnabled(a))) || (!init_val && glIsEnabled(a))) \
  ogEnvLog(OG_LFAIL, "%s incorrect initial condition.\n", #a); \
glEnable(a); \
if (!glIsEnabled(a)) { \
  ogEnvLog(OG_LFAIL, "%s should be enabled but isn't.\n", #a); \
}  \
glDisable(a); \
if (glIsEnabled(a)) { \
  ogEnvLog(OG_LFAIL, "%s shouldn't be enabled but is.\n", #a); \
}

static int maybe_enable(GLenum what)
{
  if (ogLibBitRand(1)) {
    glEnable(what);
    return 1;
  } else return 0;
}

static void do_verify_int(char *name, GLenum what, GLint val, int hex)
{
    GLint set_val;
    glGetIntegerv(what, &set_val);
    if (set_val != val) {
	if (hex) {
	    ogEnvLog(OG_LFAIL, "%s is 0x%x, should be 0x%x\n", 
		     name, set_val, val);
	} else {
	    ogEnvLog(OG_LFAIL, "%s is %d, should be %d\n", 
		     name, set_val, val);
	}
    }
}

static void do_verify_pointer(char *name, GLenum what, GLvoid *val)
{
  GLvoid *set_val;
  GetPointervEXT(what, &set_val);
  if (set_val != val) {
	ogEnvLog(OG_LFAIL, "%s is %x, should be %x\n", name, set_val, val);
  }
}

static void do_verify_floatv(char *name, GLenum what, GLfloat *val, int size)
{
  char buf1[64], buf2[64];
  GLfloat set_val[4];
  char *format;
  int i;

  switch(size) {
  case 1: format = "( %f )"; break;
  case 2: format = "( %f %f )"; break;
  case 3: format = "( %f %f %f )"; break;
  case 4: format = "( %f %f %f %f )"; break;
  default: assert(0);
  }

  glGetFloatv(what, set_val);
  for (i = 0; i < size; i++) {
    if (fabs(val[i] - set_val[i]) > ogLibGetEtol()) {
      sprintf(buf1, format, set_val[0], set_val[1], set_val[2], set_val[3]);
      sprintf(buf2, format, val[0], val[1], val[2], val[3]);
      ogEnvLog(OG_LFAIL, "%s is %s, should be %s\n", name, buf1, buf2);
      break;
    }
  }
}

static void do_verify_bool(char *name, GLenum what, GLboolean val)
{
  GLboolean set_val;

  glGetBooleanv(what, &set_val);
  if ((!set_val && !val) || (set_val && val)) return;
  ogEnvLog(OG_LFAIL, "%s should %sbe set, but is%s\n", name,
	   val ? "" : "not ", set_val ? "" : "n't");
}

static void do_set_random_floatv(GLvoid (*func)(GLfloat *))
{
    float f[4];
    f[0] = ogLibFloatRand(0, 1);
    f[1] = ogLibFloatRand(0, 1);
    f[2] = ogLibFloatRand(0, 1);
    f[3] = ogLibFloatRand(0, 1);
    (*func)(f);
}

static void print_enabled(const char *name, int enabled) 
{
  ogEnvLog(1, "%s %s enabled.\n", name, enabled ? "" : "not");
}

static void print_type(const char *name, GLenum type, int enabled) 
{
  const char *ctype;

  if (!enabled) return;
  switch(type) {
  case GL_BYTE: ctype = "BYTE"; break;
  case GL_UNSIGNED_BYTE: ctype = "UNSIGNED_BYTE"; break;
  case GL_SHORT: ctype = "SHORT"; break;
  case GL_UNSIGNED_SHORT: ctype = "UNSIGNED_SHORT"; break;
  case GL_INT: ctype = "INT"; break;
  case GL_UNSIGNED_INT: ctype = "UNSIGNED_INT"; break;
  case GL_FLOAT: ctype = "FLOAT"; break;
  case GL_DOUBLE_EXT: ctype = "DOUBLE_EXT"; break;
  default:
    assert(0);
    break;
  }
  ogEnvLog(1, "%s:  %s\n", name, ctype);
}

static void print_size(const char *name, GLint size, int enabled)
{
  if (!enabled) return;
  ogEnvLog(1, "%s:  %d\n", name, size);
}

static void print_stride(const char *name, GLint stride, int enabled)
{
  if (!enabled) return;
  ogEnvLog(1, "GL_%s_ARRAY_STRIDE_EXT:  %d\n", name, stride);
}

#endif

/*ARGSUSED*/
TESTMOD(extvertarray)
{

#ifdef GL_DOUBLE_EXT
  GLboolean rgbMode;
  GLint float_only;
  /* three for the strides, max_size for how many, four for the components.
   * doubles since that is the biggest type and the one with the strongest
   * alignment requirements */
  GLdouble vn[3*MAX_SIZE * 4];
  GLdouble cn[3*MAX_SIZE * 4];
  GLdouble tn[3*MAX_SIZE * 4];
  GLdouble nn[3*MAX_SIZE * 3];
  GLboolean en[3*MAX_SIZE];
  unsigned char *vnumbers = (unsigned char *)vn, 
  *cnumbers = (unsigned char *)cn, *inumbers = (unsigned char *)cn,
  *nnumbers = (unsigned char *)nn, *tnumbers = (unsigned char *)tn,
  *enumbers = (unsigned char *)en;
  GLfloat vertices[MAX_SIZE][4];
  GLuint colors[MAX_SIZE];
  /* normal is 4 long for convenience */
  GLfloat set_vertex[4], set_normal[4], set_color[4], set_texcoord[4], 
  set_index;
  GLfloat cruft_normal[4], cruft_color[4], cruft_texcoord[4], cruft_index;
  GLboolean set_edge;
  GLboolean cruft_edge;
  GLuint set_lib_color;
  unsigned int xmax, ymax;
  unsigned int max_color_index;
  int elem;
  int i;
  GLvoid (*tfloatfunc)(const GLfloat *);
  GLint screen_max;
  GLint first;
  GLsizei count;
  GLint obj;

  int venab, nenab, cenab, ienab, tenab, eenab;
  GLsizei vsize, csize, tsize;
  GLsizei vcount, ncount, ccount, icount, tcount, ecount;
  GLenum vtype, ntype, ctype, itype, ttype;
  GLvoid (__stdcall*vfunc)(const GLvoid *);
  GLvoid (__stdcall*nfunc)(const GLvoid *);
  GLvoid (__stdcall*cfunc)(const GLvoid *);
  GLvoid (__stdcall*ifunc)(const GLvoid *);
  GLvoid (__stdcall*tfunc)(const GLvoid *);
  int vstride, nstride, cstride, istride, tstride, estride;
  int set_vstride, set_nstride, set_cstride, set_istride, set_tstride, 
  set_estride;

  SUPPORTED_EXTENSION("GL_EXT_vertex_array");

  ArrayElementEXT = (PFNGLARRAYELEMENTEXTPROC) wglGetProcAddress("glArrayElementEXT");
  ColorPointerEXT = (PFNGLCOLORPOINTEREXTPROC) wglGetProcAddress("glColorPointerEXT");
  DrawArraysEXT = (PFNGLDRAWARRAYSEXTPROC) wglGetProcAddress("glDrawArraysEXT");
  EdgeFlagPointerEXT = (PFNGLEDGEFLAGPOINTEREXTPROC) wglGetProcAddress("glEdgeFlagPointerEXT");
  GetPointervEXT = (PFNGLGETPOINTERVEXTPROC) wglGetProcAddress("glGetPointervEXT");
  IndexPointerEXT = (PFNGLINDEXPOINTEREXTPROC) wglGetProcAddress("glIndexPointerEXT");
  NormalPointerEXT = (PFNGLNORMALPOINTEREXTPROC) wglGetProcAddress("glNormalPointerEXT");
  TexCoordPointerEXT = (PFNGLTEXCOORDPOINTEREXTPROC) wglGetProcAddress("glTexCoordPointerEXT");
  VertexPointerEXT = (PFNGLVERTEXPOINTEREXTPROC) wglGetProcAddress("glVertexPointerEXT");

  /* make sure that all the little dots will be inside the window */
  xmax = ogEnvQuery(OG_XWSIZE) - 1;
  ymax = ogEnvQuery(OG_YWSIZE) - 1;
  screen_max = MAX_SIZE;
  if (xmax/W - 5 < screen_max) screen_max = xmax/W - 5;
  if (ymax/H - 5 < screen_max) screen_max = ymax/H - 5;

  glMatrixMode(GL_PROJECTION);
  glOrtho(.5, xmax, .5, ymax, -1, 1);

  glGetBooleanv(GL_RGBA_MODE, &rgbMode);
  if (!rgbMode) {
      max_color_index = ogEnvCurVisualInfo(GLX_BUFFER_SIZE);
      max_color_index = (1 << max_color_index) - 1;
  }

  while (pass--) {
    /* Do a quick run-through of the enables */
    TEST_ENABLE(GL_VERTEX_ARRAY_EXT, FALSE);
    TEST_ENABLE(GL_NORMAL_ARRAY_EXT, FALSE);
    TEST_ENABLE(GL_COLOR_ARRAY_EXT, FALSE);
    TEST_ENABLE(GL_INDEX_ARRAY_EXT, FALSE);
    TEST_ENABLE(GL_TEXTURE_COORD_ARRAY_EXT, FALSE);
    TEST_ENABLE(GL_EDGE_FLAG_ARRAY_EXT, FALSE);

    /* choose which element to draw */
    elem = ogLibIntRand(0, screen_max);

    /* choose sizes */
    vsize = ogLibIntRand(2, 4);
    csize = ogLibIntRand(3, 4);
    tsize = ogLibIntRand(2, 4);

    /* choose indices of types for the various things.  1/5th of the time
     * we'll make everything an int since those are the special cases and 
     * we want to exercise them. */
    if (ogLibIntRand(0, 1)) {
      ogEnvLog(1, "using mixed types\n");
      float_only = 0;
      vtype = ogLibIntRand(0, vtype_count - 1);
      ntype = ogLibIntRand(0, ntype_count - 1);
      ctype = ogLibIntRand(0, ctype_count - 1);
      itype = ogLibIntRand(0, itype_count - 1);
      ttype = ogLibIntRand(0, ttype_count - 1);
    } else {
      ogEnvLog(1, "using floats only\n");
      float_only = 1;
      vtype = 2;
      ntype = 3;
      ctype = 6;
      itype = 2;
      ttype = 2;
    }

    /* figure out function pointers */
    vfunc = vfuncs[(vsize - 2) * vtype_count + vtype];
    nfunc = nfuncs[ntype];
    cfunc = cfuncs[(csize - 3) * ctype_count + ctype];
    ifunc = ifuncs[itype];
    tfunc = tfuncs[(tsize - 1) * ttype_count + ttype];

    /* map indices of types to types */
    vtype = vtypes[vtype];
    ntype = ntypes[ntype];
    ctype = ctypes[ctype];
    itype = itypes[itype];
    ttype = ttypes[ttype];
	
    if (float_only) {
      assert(vtype == GL_FLOAT);
      assert(ntype == GL_FLOAT);
      assert(itype == GL_FLOAT);
      assert(ttype == GL_FLOAT);
    }

    /* choose strides.  1 in 3 chance that any given stride will be set to 0 */
    if (ogLibIntRand(0, 2)) {
	set_vstride = vstride = vsize * enum_size(vtype) * 
	    ogLibIntRand(1, 3);
    } else {
	vstride = vsize * enum_size(vtype);
	set_vstride = 0;
    }
    if (ogLibIntRand(0, 2)) {
	set_nstride = nstride = 3 * enum_size(ntype) * 
	    ogLibIntRand(1, 3);
    } else {
	nstride = 3 * enum_size(ntype);
	set_nstride = 0;
    }
    if (ogLibIntRand(0, 2)) {
	set_cstride = cstride = csize * enum_size(ctype) * 
	    ogLibIntRand(1, 3);
    } else {
	cstride = csize * enum_size(ctype);
	set_cstride = 0;
    }
    if (ogLibIntRand(0, 2)) {
	set_istride = istride = 1 * enum_size(itype) * 
	    ogLibIntRand(1, 3);
    } else {
	istride = enum_size(itype);
	set_istride = 0;
    }
    if (ogLibIntRand(0, 2)) {
	set_tstride = tstride = tsize * enum_size(ttype) * 
	    ogLibIntRand(1, 3);
    } else {
	tstride = tsize * enum_size(ttype);
	set_tstride = 0;
    }
    if (ogLibIntRand(0, 2)) {
	set_estride = estride = enum_size(GL_BYTE) * 
	    ogLibIntRand(1, 3);
    } else {
	estride = enum_size(GL_BYTE);
	set_estride = 0;
    }

    /* choose counts */
    vcount = ogLibIntRand(0, MAX_SIZE);
    ncount = ogLibIntRand(0, MAX_SIZE);
    ccount = ogLibIntRand(0, MAX_SIZE);
    icount = ogLibIntRand(0, MAX_SIZE);
    tcount = ogLibIntRand(0, MAX_SIZE);
    ecount = ogLibIntRand(0, MAX_SIZE);

    /* set up the arrays */
    {
      unsigned char *vp = vnumbers, *cp = cnumbers, *ip = cnumbers,
      *np = nnumbers, *tp = tnumbers, *ep = enumbers;
      float tmp[4];
      for (i = 0; i < MAX_SIZE; i++) {
	if (rgbMode)
	   glColor4f(ogLibFloatRand(0, 1), ogLibFloatRand(0, 1),
		     ogLibFloatRand(0, 1), ogLibFloatRand(0, 1));
	else 
	   glIndexf(ogLibFloatRand(0, max_color_index));

	/* to read the color into the array we get it as a float then use 
	 * SetReadSet to get it into the array */
	if (rgbMode) {
	  glGetFloatv(GL_CURRENT_COLOR, &tmp[0]);
	  ogLibSetReadSet(glColor4fv, cfunc, GL_CURRENT_COLOR, ctype,
			  &tmp[0], cp, 4, 1);
	} else {
	  glGetFloatv(GL_CURRENT_INDEX, &tmp[0]);
	  ogLibSetReadSet(glIndexfv, ifunc, GL_CURRENT_INDEX, itype, 
			  &tmp[0], ip, 1, 0);
	}
	draw_read_color(&colors[i]);
	
	glTexCoord4f(ogLibFloatRand(0, 1), ogLibFloatRand(0, 1),
		     ogLibFloatRand(0, 1), ogLibFloatRand(0, 1));
	glGetFloatv(GL_CURRENT_TEXTURE_COORDS, &tmp[0]);
	ogLibSetReadSet(glTexCoord4fv, tfunc, GL_CURRENT_TEXTURE_COORDS,
			ttype, &tmp[0], tp, tsize, 0);

	glNormal3f(ogLibFloatRand(0, 1), ogLibFloatRand(0, 1),
		   ogLibFloatRand(0, 1));
	glGetFloatv(GL_CURRENT_NORMAL, &tmp[0]);
	ogLibSetReadSet(glNormal3fv, nfunc, GL_CURRENT_NORMAL,
			ntype, &tmp[0], np, 3, 0);
		     
	ep[0] = ogLibIntRand(0, 2);

		       
	vertices[i][0] = i*W + W;
	vertices[i][1] = i*H + H;
	vertices[i][2] = 0;
	vertices[i][3] = 1;
	ogLibSetReadSet(glRasterPos4fv, vfunc, GL_CURRENT_RASTER_POSITION,
			vtype, vertices[i], vp, 4, 0);
	cp += cstride;
	ip += istride;
	vp += vstride;
	tp += tstride;
	np += nstride;
	ep += estride;
      }
      assert(cp <= cnumbers + sizeof(cn));
      assert(ip <= cnumbers + sizeof(cn));
      assert(vp <= vnumbers + sizeof(vn));
      assert(tp <= tnumbers + sizeof(tn));
      assert(np <= nnumbers + sizeof(nn));
      assert(ep <= enumbers + sizeof(en));
    }

    /* enable / don't enable various array things */
    START_DL_OR_IM(1);
    venab = ogLibIntRand(0, 10);
    if (venab) glEnable(GL_VERTEX_ARRAY_EXT);
    nenab = maybe_enable(GL_NORMAL_ARRAY_EXT);
    if (rgbMode) {
      cenab = maybe_enable(GL_COLOR_ARRAY_EXT);
      ienab = 0;
    } else {
      ienab = maybe_enable(GL_INDEX_ARRAY_EXT);
      cenab = 0;
    }
    tenab = maybe_enable(GL_TEXTURE_COORD_ARRAY_EXT);
    if (!float_only) eenab = maybe_enable(GL_EDGE_FLAG_ARRAY_EXT);
    else {
      eenab = 0;
      glDisable(GL_EDGE_FLAG_ARRAY_EXT);
    }
    FINIS_DL_OR_IM(1);

    /* print everything out */
    print_enabled("VERTEX", venab);
    print_type("vertex type", vtype, venab);
    print_size("vertex size", vsize, venab);
    print_stride("VERTEX", set_vstride, venab);

    print_enabled("NORMAL", nenab);
    print_type("normal type", ntype, nenab);
    print_stride("NORMAL", set_nstride, nenab);

    print_enabled("COLOR", cenab);
    print_type("color type", ctype, cenab);
    print_size("color size", csize, cenab);
    print_stride("COLOR", set_cstride, cenab);

    print_enabled("INDEX", ienab);
    print_type("index type", itype, ienab);
    print_stride("INDEX", set_istride, ienab);

    print_enabled("TEXTURE_COORD", tenab);
    print_type("texture_coord type", ttype, tenab);
    print_size("texture_coord size", tsize, tenab);
    print_stride("TEXTURE_COORD", set_tstride, tenab);

    print_enabled("EDGE_FLAG", eenab);
    print_stride("EDGE_FLAG", set_estride, eenab);

    /* here we figure out what the state should be when we're done.  if the
     * arrays are enabled we get the value from them and read it back, then
     * set the value to something random.  otherwise we create and remember
     * a random value to which we set the state.  either way, the value
     * of the parameter should be set_foo once we've done the element --
     * meaning that it has been changed to the proper value or it hasn't 
     * been changed from the proper value. */
    if (venab) {
	/* this is a little odd.  we're just using vfunc to do the type 
	 * conversion -- it's easier than figuring it out for ourselves.
	 * (vfunc is actually glRasterPos) */
	ogLibSetRead(vfunc, GL_CURRENT_RASTER_POSITION, 
		     set_vertex, vnumbers + elem*vstride);
    }
    if (nenab) {
	ogLibSetRead(nfunc, GL_CURRENT_NORMAL, 
		     set_normal, nnumbers + elem*nstride);
	/* set the normal to be total cruft */
	do_set_random_floatv((GLvoid (*)(GLfloat *))glNormal3fv);
	glGetFloatv(GL_CURRENT_NORMAL, cruft_normal);
    } else {
	set_normal[0] = ogLibFloatRand(-1, 1);
	set_normal[2] = ogLibFloatRand(-1, 1);
	set_normal[3] = ogLibFloatRand(-1, 1);
	glNormal3fv(set_normal);
    }
    if (rgbMode) {
	if (cenab) {
	    ogLibSetRead(cfunc, GL_CURRENT_COLOR,
			 set_color, cnumbers + elem*cstride);
	    draw_read_color(&set_lib_color);
	    do_set_random_floatv((GLvoid (*)(GLfloat *))
				 ((csize == 3) ? glColor3fv : glColor4fv));
	    glGetFloatv(GL_CURRENT_COLOR, cruft_color);
	} else {
	    glColor4f(ogLibFloatRand(0, 1), ogLibFloatRand(0, 1),
		      ogLibFloatRand(0, 1), ogLibFloatRand(0, 1));
	    glGetFloatv(GL_CURRENT_COLOR, &set_color[0]);
	    draw_read_color(&set_lib_color);
	}
    }
    /* need a function pointer to set the tex coord... */
    switch(tsize) {
      case 1: tfloatfunc = glTexCoord1fv; break;
      case 2: tfloatfunc = glTexCoord2fv; break;
      case 3: tfloatfunc = glTexCoord3fv; break;
      case 4: tfloatfunc = glTexCoord4fv; break;
      default: assert(0);
    }
    if (tenab) {
	ogLibSetRead(tfunc, GL_CURRENT_TEXTURE_COORDS,
		     set_texcoord, tnumbers + elem*tstride);
	do_set_random_floatv((GLvoid (*)(GLfloat *))tfloatfunc);
	glGetFloatv(GL_CURRENT_TEXTURE_COORDS, cruft_texcoord);
    } else {
	set_texcoord[0] = ogLibFloatRand(-5, 5);
	set_texcoord[1] = ogLibFloatRand(-5, 5);
	set_texcoord[2] = ogLibFloatRand(-5, 5);
	set_texcoord[3] = ogLibFloatRand(-5, 5);
	(*tfloatfunc)(set_texcoord);
    }
    if (!rgbMode) {
	if (ienab) {
	    ogLibSetRead(ifunc, GL_CURRENT_INDEX,
			 &set_index, inumbers + elem*istride);
	    draw_read_color(&set_lib_color);
	    glIndexi(ogLibIntRand(0, max_color_index));
	    glGetFloatv(GL_CURRENT_INDEX, &cruft_index);
	} else {
	    set_index = ogLibIntRand(0, max_color_index);
	    glIndexf(set_index);
	    draw_read_color(&set_lib_color);
	}
    }
    if (eenab) {
	glEdgeFlagv(&enumbers[elem*estride]);
	glGetBooleanv(GL_EDGE_FLAG, &set_edge);
	glEdgeFlag(ogLibBitRand(1));
	glGetBooleanv(GL_EDGE_FLAG, &cruft_edge);
    } else {
	set_edge = ogLibBitRand(1);
	glEdgeFlag(set_edge);
    }

    /* set up the vertices.  if something is not enabled, either do not call the
     * setup routine at all or set it to zero.  this makes sure that a pointer
     * which does not correspond to an enabled array is not dereferenced and that
     * the default setup is correct. */
    if (venab) {
	VertexPointerEXT(vsize, vtype, vstride, vcount, (GLvoid *)vnumbers);
    } else if (ogLibBitRand(1)) {
	VertexPointerEXT(vsize, vtype, vstride, vcount, (GLvoid *)0);
    }
    if (nenab) {
	NormalPointerEXT(ntype, nstride, ncount, (GLvoid *)nnumbers);
    } else if (ogLibBitRand(1)) {
	NormalPointerEXT(ntype, nstride, ncount, (GLvoid *)0);
    }
    if (cenab) {
	ColorPointerEXT(csize, ctype, cstride, ccount, (GLvoid *)cnumbers);
    } else if (ogLibBitRand(1)) {
	ColorPointerEXT(csize, ctype, cstride, ccount, (GLvoid *)0);

    }
    if (ienab) {
	IndexPointerEXT(itype, istride, icount, (GLvoid *)inumbers);
    } else if (ogLibBitRand(1)) {
	IndexPointerEXT(itype, istride, icount, (GLvoid *)0);
    }
    if (tenab) {
	TexCoordPointerEXT(tsize, ttype, tstride, tcount, (GLvoid *)tnumbers);
    } else if (ogLibBitRand(1)) {
	TexCoordPointerEXT(tsize, ttype, tstride, tcount, (GLvoid *)0);
    }
	EdgeFlagPointerEXT(estride, ecount, enumbers);
    
    glPointSize(3);
    START_DL_OR_IM(1);
    ogLibClear(0); /*glClear(GL_COLOR_BUFFER_BIT); */
    glBegin(GL_POINTS);
    ogEnvLog(1, "glArrayElementEXT(%d)\n", elem);
    ArrayElementEXT(elem);
    glEnd();
    FINIS_DL_OR_IM(1);
    glPointSize(1);

    /* see if the point actually got drawn */
    if (venab) {
	if (ogLibPixelCheck(vertices[elem][0], vertices[elem][1], 
			    (cenab || ienab) ? colors[elem] : set_lib_color)) {
	    /* I need to get the debugger to stop here... */
	    printf("");
	}
    }

    /* see if things are set to what they should be set to */
    do_verify_floatv("CURRENT_NORMAL", GL_CURRENT_NORMAL, set_normal, 3);
    if (rgbMode) 
      do_verify_floatv("CURRENT_COLOR", GL_CURRENT_COLOR, set_color, csize);
    else 
      do_verify_floatv("CURRENT_INDEX", GL_CURRENT_INDEX, &set_index, 1);
    do_verify_floatv("CURRENT_TEXTURE_COORDS", 
		     GL_CURRENT_TEXTURE_COORDS, set_texcoord, tsize);
    do_verify_bool("EDGE_FLAG", GL_EDGE_FLAG, set_edge);

    /* try to draw a series of points */
    first = ogLibIntRand(0, screen_max);
    count = ogLibIntRand(0, screen_max - first);

    ogLibClear(0); /*glClear(GL_COLOR_BUFFER_BIT); */

    /* draw with count == 0 just to make sure nothing bad happens */
    START_DL_OR_IM(1);
    ogEnvLog(1, "glDrawArraysEXT(GL_POINTS, %d, 0)\n", first);
    DrawArraysEXT(GL_POINTS, first, 0);
    FINIS_DL_OR_IM(1);

    glPointSize(3);

    START_DL_OR_IM(1);
    ogEnvLog(1, "glDrawArraysEXT(GL_POINTS, %d, %d)\n", first, count);
    DrawArraysEXT(GL_POINTS, first, count);
    FINIS_DL_OR_IM(1);

    glPointSize(1);

    /* make sure the points are actually there */
    if (venab) {
	for (i = 0; i < count; i++) {
	    if (ogLibPixelCheck(vertices[first + i][0], vertices[first + i][1],
				(cenab || ienab) ? colors[first+i] : 
				set_lib_color))
	      /* I need to get the debugger to stop here... */
	      printf("");
	}
    }

    /* make sure that the points before and after aren't there */
    if (first) {
	ogLibPixelCheck((first - 1) * W, (first - 1) * H, 0);
    }
    if (count < MAX_SIZE - first) {
	ogLibPixelCheck((first + count + 1) * W, (first + count + 1) * H, 0);
    }

    /* see if things are set to what they should be.  since state is
     * undefined if something is enabled, don't check that */
    if (!nenab) 
      do_verify_floatv("CURRENT_NORMAL", GL_CURRENT_NORMAL, set_normal, 3);
    if (!cenab && rgbMode)
      do_verify_floatv("CURRENT_COLOR", GL_CURRENT_COLOR, set_color, csize);
    if (!ienab && !rgbMode)
      do_verify_floatv("CURRENT_INDEX", GL_CURRENT_INDEX, &set_index, 1);
    if (!tenab)
      do_verify_floatv("CURRENT_TEXTURE_COORDS", GL_CURRENT_TEXTURE_COORDS,
		       set_texcoord, tsize);
    if (!eenab)
      do_verify_bool("EDGE_FLAG", GL_EDGE_FLAG, set_edge);

    /* test get commands */
    if (venab) {
	do_verify_int("vsize", GL_VERTEX_ARRAY_SIZE_EXT, vsize, 0);
	do_verify_int("vtype", GL_VERTEX_ARRAY_TYPE_EXT, vtype, 1);
	do_verify_int("vstride", GL_VERTEX_ARRAY_STRIDE_EXT, vstride, 0);
	do_verify_int("vcount", GL_VERTEX_ARRAY_COUNT_EXT, vcount, 0);
	do_verify_pointer("vpointer", GL_VERTEX_ARRAY_POINTER_EXT, 
			  (GLvoid *)vnumbers);
    }

    if (nenab) {
	do_verify_int("ntype", GL_NORMAL_ARRAY_TYPE_EXT, ntype, 1);
	do_verify_int("nstride", GL_NORMAL_ARRAY_STRIDE_EXT, nstride, 0);
	do_verify_int("ncount", GL_NORMAL_ARRAY_COUNT_EXT, ncount, 0);
	do_verify_pointer("npointer", GL_NORMAL_ARRAY_POINTER_EXT, nnumbers);
    }

    if (cenab) {
	do_verify_int("csize", GL_COLOR_ARRAY_SIZE_EXT, csize, 0);
	do_verify_int("ctype", GL_COLOR_ARRAY_TYPE_EXT, ctype, 1);
	do_verify_int("cstride", GL_COLOR_ARRAY_STRIDE_EXT, cstride, 0);
	do_verify_int("ccount", GL_COLOR_ARRAY_COUNT_EXT, ccount, 0);
	do_verify_pointer("npointer", GL_COLOR_ARRAY_POINTER_EXT, 
			  (GLvoid *)cnumbers);
    }

    if (ienab) {
	do_verify_int("itype", GL_INDEX_ARRAY_TYPE_EXT, itype, 1);
	do_verify_int("istride", GL_INDEX_ARRAY_STRIDE_EXT, istride, 0);
	do_verify_int("icount", GL_INDEX_ARRAY_COUNT_EXT, icount, 0);
	do_verify_pointer("ipointer", GL_INDEX_ARRAY_POINTER_EXT,
			  (GLvoid *)inumbers);
    }

    if (tenab) {
	do_verify_int("tsize", GL_TEXTURE_COORD_ARRAY_SIZE_EXT, tsize, 0);
	do_verify_int("ttype", GL_TEXTURE_COORD_ARRAY_TYPE_EXT, ttype, 1);
	do_verify_int("tstride", GL_TEXTURE_COORD_ARRAY_STRIDE_EXT, tstride, 
		      0);
	do_verify_int("tcount", GL_TEXTURE_COORD_ARRAY_COUNT_EXT, tcount, 0);
	do_verify_pointer("tpointer", GL_TEXTURE_COORD_ARRAY_POINTER_EXT,
			  tnumbers);
    }

    if (eenab) {
	do_verify_int("estride", GL_EDGE_FLAG_ARRAY_STRIDE_EXT, estride, 0);
	do_verify_int("ecount", GL_EDGE_FLAG_ARRAY_COUNT_EXT, ecount, 0);
	do_verify_pointer("epointer", GL_EDGE_FLAG_ARRAY_POINTER_EXT,
			  enumbers);
    }

#if 0
    /* this part is not finished -- it draws, but doesn't check the
     * results */

    /* test to make sure that we can draw something besides
     * points.  pick a spot in the vertex array and replace 4 points with 
     * the corners of a rectangle.  then draw the rectangle in some mode and 
     * see it things worked. */
    {
      GLint corner = ogLibIntRand(0, screen_max - 5);
      GLfloat vertex[4];
      unsigned int color;
      GLenum modes[] = {GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_TRIANGLE_STRIP, 
			    GL_TRIANGLE_FAN, GL_TRIANGLES, GL_QUAD_STRIP, GL_QUADS, GL_POLYGON};
      GLenum mode = modes[ogLibIntRand(0, 9)];

      vertex[0] = xmax / 8 * 3;
      vertex[1] = ymax / 8 * 3;
      vertex[2] = 0;
      vertex[3] = 1;

      ogLibSetReadSet(glRasterPos4fv, vfunc, GL_CURRENT_RASTER_POSITION,
		      vtype, &vertex[0], vnumbers + vstride*corner, 4, 0);
      vertex[0] += xmax / 4;
      ogLibSetReadSet(glRasterPos4fv, vfunc, GL_CURRENT_RASTER_POSITION,
		      vtype, &vertex[0], vnumbers + vstride*(corner+1), 4, 0);
      vertex[1] += ymax / 4;
      ogLibSetReadSet(glRasterPos4fv, vfunc, GL_CURRENT_RASTER_POSITION,
		      vtype, &vertex[0], vnumbers + vstride*(corner+2), 4, 0);
      vertex[0] -= xmax / 4;
      ogLibSetReadSet(glRasterPos4fv, vfunc, GL_CURRENT_RASTER_POSITION,
		      vtype, &vertex[0], vnumbers + vstride*(corner+3), 4, 0);

      /* keep it simple -- we'll draw the entire rectangle in a given color
       * instead of being tricky and using the colors in the array.  that 
       * should have been tested up above */
      ogLibClear(0);
      glDisable(GL_COLOR_ARRAY_EXT);
      glDisable(GL_INDEX_ARRAY_EXT);
      color = ogLibColor();

      glLineWidth(4);
      glPointSize(3);

      START_DL_OR_IM(1);
      ogEnvLog(1, "glDrawArraysEXT(%d, %d, 4)\n", mode, corner);
      DrawArraysEXT(mode, corner, 4);
      FINIS_DL_OR_IM(1);

      glPointSize(1);
      glLineWidth(1);

      /* check what we drew based on what the mode is */
      switch(mode) {
	case GL_POINTS:
	  /* check the corners */
	  break;
	case GL_LINE_STRIP:  case GL_LINE_LOOP:  case GL_LINES:
	  /* check the lines on the top & bottom */
	  break;
	case GL_TRIANGLE_STRIP:
	  break;
	case GL_QUADS: case GL_POLYGON:
	  /* check the rectangle */
	  break;
      default: 
	break;
      }
    }
#endif


    glDisable(GL_VERTEX_ARRAY_EXT);
    glDisable(GL_NORMAL_ARRAY_EXT);
    glDisable(GL_COLOR_ARRAY_EXT);
    glDisable(GL_INDEX_ARRAY_EXT);
    glDisable(GL_TEXTURE_COORD_ARRAY_EXT);
    glDisable(GL_EDGE_FLAG_ARRAY_EXT);      
  }

#endif

}

CLEANUP(extvertarray)
{
#ifdef GL_DOUBLE_EXT
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glTexCoord4i(0, 0, 0, 1);
    glNormal3f(0, 0, 1);
    glEdgeFlag(1);
    ogLibSetDefaultColors();
    ogLibSetDefaultClears();
    ogLibSetDefaultRasterPos();
    VertexPointerEXT(4, GL_FLOAT, 0, 0, 0);
    NormalPointerEXT(GL_FLOAT, 0, 0, 0);
    ColorPointerEXT(4, GL_FLOAT, 0, 0, 0);
    TexCoordPointerEXT(4, GL_FLOAT, 0, 0, 0);
    IndexPointerEXT(GL_FLOAT, 0, 0, 0);
    EdgeFlagPointerEXT(0, 0, 0);
#endif
}
  

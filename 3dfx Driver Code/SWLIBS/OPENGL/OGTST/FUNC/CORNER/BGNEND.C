#include <stdio.h>
#include "ogtst.h"

#define TESTIT(tag,x)   ogEnvLog(1, "Testing %s\n", tag); glBegin(GL_POINTS); x; glEnd();  check_error(##tag);
	
static void
check_error(const char *tag) {
    GLint e;

    ogEnvLog(3, "%s\n", tag);
    if ((e = glGetError()) != GL_INVALID_OPERATION) {
	ogEnvLog(OG_LFAIL, "%s failed with error 0x%x\n", tag, e);
	while (e != GL_NO_ERROR) {
	    if ((e = glGetError()) != GL_NO_ERROR)
		ogEnvLog(OG_LFAIL, "%s also received error 0x%x\n", tag, e);
	}
    }
}


/*ARGSUSED*/
TESTMOD(bgnend) {
  GLboolean bv[4];
  GLdouble dv[16];
  GLfloat fv[16];
  GLfloat ffv[129];
  GLint iv[16];
  GLuint uiv[4];
  GLshort sv[4];
  GLushort usv[4];
  GLubyte stipple[128];
  GLubyte conv[128], row[128], col[128], hist[128];
  const GLubyte *ex;
  int hwtype = ogEnvQuery(OG_HW);
  GLint e;

  glNewList(1, GL_COMPILE);
  glColor3f(0.,0.,0.);
  glEndList();

  TESTIT("glBegin", glBegin(GL_POINTS));
  TESTIT("glEnd", glEnd());
  TESTIT("glNewList", glNewList(2, GL_COMPILE));
  TESTIT("glEndList", glEndList());
  TESTIT("glDeleteLists", glDeleteLists(1,1));
  TESTIT("glGenLists", glGenLists(5));
  TESTIT("glListBase", glListBase(1));
  TESTIT("glBegin", glBegin(GL_QUADS));
  TESTIT("glBitmap", glBitmap(0,0,0.,0.,10,10,NULL));
  TESTIT("glRasterPos2d", glRasterPos2d(1.,1.));
  TESTIT("glRasterPos2dv", glRasterPos2dv(dv));
  TESTIT("glRasterPos2f", glRasterPos2f(1.,1.));
  TESTIT("glRasterPos2fv", glRasterPos2fv(fv));
  TESTIT("glRasterPos2i", glRasterPos2i(1,1));
  TESTIT("glRasterPos2iv", glRasterPos2iv(iv));
  TESTIT("glRasterPos2s", glRasterPos2s(1,1));
  TESTIT("glRasterPos2sv", glRasterPos2sv(sv));
  TESTIT("glRasterPos3d", glRasterPos3d(1.,1.,1.));
  TESTIT("glRasterPos3dv", glRasterPos3dv(dv));
  TESTIT("glRasterPos3f", glRasterPos3f(1.,1.,1.));
  TESTIT("glRasterPos3fv", glRasterPos3fv(fv));
  TESTIT("glRasterPos3i", glRasterPos3i(1,1,1));
  TESTIT("glRasterPos3iv", glRasterPos3iv(iv));
  TESTIT("glRasterPos3s", glRasterPos3s(1,1,1));
  TESTIT("glRasterPos3sv", glRasterPos3sv(sv));
  TESTIT("glRasterPos4d", glRasterPos4d(1.,1.,1.,1.));
  TESTIT("glRasterPos4dv", glRasterPos4dv(dv));
  TESTIT("glRasterPos4f", glRasterPos4f(1.,1.,1.,1.));
  TESTIT("glRasterPos4fv", glRasterPos4fv(fv));
  TESTIT("glRasterPos4i", glRasterPos4i(1,1,1,1));
  TESTIT("glRasterPos4iv", glRasterPos4iv(iv));
  TESTIT("glRasterPos4s", glRasterPos4s(1,1,1,1));
  TESTIT("glRasterPos4sv", glRasterPos4sv(sv));
  TESTIT("glRectd", glRectd(0.,0.,1.,1.));
  TESTIT("glRectdv", glRectdv(dv,dv));
  TESTIT("glRectf", glRectf(0.,0.,1.,1.));
  TESTIT("glRectfv", glRectfv(fv,fv));
  TESTIT("glRecti", glRecti(0,0,1,1));
  TESTIT("glRectiv", glRectiv(iv,iv));
  TESTIT("glRects", glRects(0,0,1,1));
  TESTIT("glRectsv", glRectsv(sv,sv));
  TESTIT("glClipPlane", glClipPlane(GL_CLIP_PLANE0,dv));
  TESTIT("glColorMaterial", glColorMaterial(GL_FRONT,GL_AMBIENT));
  TESTIT("glCullFace", glCullFace(GL_FRONT_AND_BACK));
  TESTIT("glFogf", glFogf(GL_FOG_MODE,GL_LINEAR));
  TESTIT("glFogfv", glFogfv(GL_FOG_MODE,fv));
  TESTIT("glFogi", glFogi(GL_FOG_MODE,GL_LINEAR));
  TESTIT("glFogiv", glFogiv(GL_FOG_MODE,iv));
  TESTIT("glFrontFace", glFrontFace(GL_CCW));
  TESTIT("glHint", glHint(GL_FOG_HINT,GL_NICEST));
  TESTIT("glLightf", glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,10.));
  TESTIT("glLightfv", glLightfv(GL_LIGHT0,GL_SPOT_EXPONENT,fv));
  TESTIT("glLighti", glLighti(GL_LIGHT0,GL_SPOT_EXPONENT,10));
  TESTIT("glLightiv", glLightiv(GL_LIGHT0,GL_SPOT_EXPONENT,iv));
  TESTIT("glLightModelf", glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,1.));
  TESTIT("glLightModelfv", glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE,fv));
  TESTIT("glLightModeli", glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1));
  TESTIT("glLightModeliv", glLightModeliv(GL_LIGHT_MODEL_TWO_SIDE,iv));
  TESTIT("glLineStipple", glLineStipple(5,0xbabe));
  TESTIT("glLineWidth", glLineWidth(2.));
  TESTIT("glPointSize", glPointSize(2.));
  TESTIT("glPolygonMode", glPolygonMode(GL_FRONT,GL_LINE));
  TESTIT("glPolygonStipple", glPolygonStipple(stipple));
  TESTIT("glScissor", glScissor(10,10,20,20));
  TESTIT("glShadeModel", glShadeModel(GL_FLAT));
  TESTIT("glTexParameterf", glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST));
  TESTIT("glTexParameterfv", glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,fv));
  TESTIT("glTexParameteri", glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST));
  TESTIT("glTexParameteriv", glTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,iv));
  TESTIT("glTexImage1D", glTexImage1D(GL_TEXTURE_1D,0,4,1,0,GL_RGBA,GL_UNSIGNED_BYTE,stipple));
  TESTIT("glTexImage2D", glTexImage2D(GL_TEXTURE_2D,0,4,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,stipple));
  TESTIT("glTexEnvf", glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_BLEND));
  TESTIT("glTexEnvfv", glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,fv));
  TESTIT("glTexEnvi", glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_BLEND));
  TESTIT("glTexEnviv", glTexEnviv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,iv));
  TESTIT("glTexGend", glTexGend(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR));
  TESTIT("glTexGendv", glTexGendv(GL_S, GL_TEXTURE_GEN_MODE, dv));
  TESTIT("glTexGenf", glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR));
  TESTIT("glTexGenfv", glTexGenfv(GL_S, GL_TEXTURE_GEN_MODE, fv));
  TESTIT("glTexGeni", glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR));
  TESTIT("glTexGeniv", glTexGeniv(GL_S, GL_TEXTURE_GEN_MODE, iv));
  TESTIT("glFeedbackBuffer", glFeedbackBuffer(4,GL_3D,fv));
  TESTIT("glSelectBuffer", glSelectBuffer(4,uiv));
  TESTIT("glRenderMode", glRenderMode(GL_FEEDBACK));
  TESTIT("glInitNames", glInitNames());
  TESTIT("glLoadName", glLoadName(0xbabe));
  TESTIT("glPassThrough", glPassThrough(0xbabe));
  TESTIT("glPopName", glPopName());
  TESTIT("glPushName", glPushName(0xbabe));
  TESTIT("glDrawBuffer", glDrawBuffer(GL_FRONT));
  TESTIT("glClear", glClear(GL_COLOR_BUFFER_BIT));
  TESTIT("glClearAccum", glClearAccum(.5,.5,.5,.5));
  TESTIT("glClearIndex", glClearIndex(2.));
  TESTIT("glClearColor", glClearColor(1.,2.,3.,4.));
  TESTIT("glClearStencil", glClearStencil(0xa));
  TESTIT("glClearDepth", glClearDepth(.5));
  TESTIT("glStencilMask", glStencilMask(0x55));
  TESTIT("glColorMask", glColorMask(GL_TRUE,GL_FALSE,GL_TRUE,GL_FALSE));
  TESTIT("glDepthMask", glDepthMask(GL_FALSE));
  TESTIT("glIndexMask", glIndexMask(0x55));
  TESTIT("glAccum", glAccum(GL_LOAD,0.5));
  TESTIT("glDisable", glDisable(GL_BLEND));
  TESTIT("glEnable", glEnable(GL_BLEND));
  TESTIT("glFinish", glFinish());
  TESTIT("glFlush", glFlush());
  TESTIT("glPopAttrib", glPopAttrib());
  TESTIT("glPushAttrib", glPushAttrib(GL_CURRENT_BIT));
  TESTIT("glMap1d", glMap1d(GL_MAP1_TEXTURE_COORD_1,1.,2.,1,2,dv));
  TESTIT("glMap1f", glMap1f(GL_MAP1_TEXTURE_COORD_1,1.,2.,1,2,fv));
  TESTIT("glMap2d", glMap2d(GL_MAP2_TEXTURE_COORD_1,1.,2.,1,2,1.,2.,1,2,dv));
  TESTIT("glMap2f", glMap2f(GL_MAP2_TEXTURE_COORD_1,1.,2.,1,2,1.,2.,1,2,fv));
  TESTIT("glMapGrid1d", glMapGrid1d(20,1.,10.));
  TESTIT("glMapGrid1f", glMapGrid1f(20,1.,10.));
  TESTIT("glMapGrid2d", glMapGrid2d(20,1.,10.,20,1.,10.));
  TESTIT("glMapGrid2f", glMapGrid2f(20,1.,10.,20,1.,10.));
  TESTIT("glEvalMesh1", glEvalMesh1(GL_LINE,1,2));
  TESTIT("glEvalMesh2", glEvalMesh2(GL_LINE,1,2,1,2));
  TESTIT("glAlphaFunc", glAlphaFunc(GL_ALWAYS,0.5));
  TESTIT("glBlendFunc", glBlendFunc(GL_ZERO,GL_ONE));
  TESTIT("glLogicOp", glLogicOp(GL_XOR));
  TESTIT("glStencilFunc", glStencilFunc(GL_ALWAYS,0.5,0x25));
  TESTIT("glStencilOp", glStencilOp(GL_INVERT,GL_INVERT,GL_INVERT));
  TESTIT("glDepthFunc", glDepthFunc(GL_ALWAYS));
  TESTIT("glPixelZoom", glPixelZoom(-1,-1));
  TESTIT("glPixelTransferf", glPixelTransferf(GL_RED_BIAS,1.));
  TESTIT("glPixelTransferi", glPixelTransferi(GL_RED_BIAS,1));
  TESTIT("glPixelStoref", glPixelStoref(GL_PACK_SWAP_BYTES,1.));
  TESTIT("glPixelStorei", glPixelStorei(GL_PACK_SWAP_BYTES,1));
  TESTIT("glPixelMapfv", glPixelMapfv(GL_PIXEL_MAP_I_TO_G,4,fv));
  TESTIT("glPixelMapuiv", glPixelMapuiv(GL_PIXEL_MAP_I_TO_G,4,uiv));
  TESTIT("glPixelMapusv", glPixelMapusv(GL_PIXEL_MAP_I_TO_G,4,usv));
  TESTIT("glReadBuffer", glReadBuffer(GL_FRONT));
  TESTIT("glCopyPixels", glCopyPixels(10,10,10,10,GL_COLOR));
  TESTIT("glReadPixels", glReadPixels(10,10,2,2,GL_RGBA,GL_UNSIGNED_BYTE,stipple));
  TESTIT("glDrawPixels", glDrawPixels(2,2,GL_RGBA,GL_UNSIGNED_BYTE,stipple));
  TESTIT("glGetBooleanv", glGetBooleanv(GL_ACCUM_ALPHA_BITS,bv));
  TESTIT("glGetClipPlane", glGetClipPlane(GL_CLIP_PLANE0, dv));
  TESTIT("glGetDoublev", glGetDoublev(GL_ACCUM_ALPHA_BITS,dv));
  TESTIT("glGetError", glGetError());
  TESTIT("glGetFloatv", glGetFloatv(GL_ACCUM_ALPHA_BITS,fv));
  TESTIT("glGetIntegerv", glGetIntegerv(GL_ACCUM_ALPHA_BITS,iv));
  TESTIT("glGetLightfv", glGetLightfv(GL_LIGHT0,GL_AMBIENT,fv));
  TESTIT("glGetLightiv", glGetLightiv(GL_LIGHT0,GL_AMBIENT,iv));
  TESTIT("glGetMapdv", glGetMapdv(GL_MAP1_TEXTURE_COORD_1,GL_COEFF,dv));
  TESTIT("glGetMapfv", glGetMapfv(GL_MAP1_TEXTURE_COORD_1,GL_COEFF,fv));
  TESTIT("glGetMapiv", glGetMapiv(GL_MAP1_TEXTURE_COORD_1,GL_COEFF,iv));
  TESTIT("glGetMaterialfv", glGetMaterialfv(GL_FRONT,GL_AMBIENT,fv));
  TESTIT("glGetMaterialiv", glGetMaterialiv(GL_FRONT,GL_AMBIENT,iv));
  TESTIT("glGetPixelMapfv", glGetPixelMapfv(GL_PIXEL_MAP_I_TO_G,fv));
  TESTIT("glGetPixelMapuiv", glGetPixelMapuiv(GL_PIXEL_MAP_I_TO_G,uiv));
  TESTIT("glGetPixelMapusv", glGetPixelMapusv(GL_PIXEL_MAP_I_TO_G,usv));
  TESTIT("glGetPolygonStipple", glGetPolygonStipple(stipple));
  TESTIT("glGetString", glGetString(GL_EXTENSIONS));
  TESTIT("glGetTexEnvfv", glGetTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,fv));
  TESTIT("glGetTexEnviv", glGetTexEnviv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,iv));
  TESTIT("glGetTexGendv", glGetTexGendv(GL_S,GL_OBJECT_PLANE,dv));
  TESTIT("glGetTexGenfv", glGetTexGenfv(GL_S,GL_OBJECT_PLANE,fv));
  TESTIT("glGetTexGeniv", glGetTexGeniv(GL_S,GL_OBJECT_PLANE,iv));
  TESTIT("glGetTexImage", glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,stipple));
  TESTIT("glGetTexParameterfv", glGetTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,fv));
  TESTIT("glGetTexParameteriv", glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,iv));
  TESTIT("glGetTexLevelParameterfv",
	 glGetTexLevelParameterfv(GL_TEXTURE_2D,0,GL_TEXTURE_COMPONENTS,fv));
  TESTIT("glGetTexLevelParameteriv",
	 glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_COMPONENTS,iv)); 
  TESTIT("glIsEnabled", glIsEnabled(GL_BLEND));
  TESTIT("glIsList", glIsList(1));
  TESTIT("glDepthRange", glDepthRange(0.25,0.75));
  TESTIT("glFrustum", glFrustum(-2.,2.,-2.,2.,0.1,1.));
  TESTIT("glLoadIdentity", glLoadIdentity());
  TESTIT("glLoadMatrixf", glLoadMatrixf(fv));
  TESTIT("glLoadMatrixd", glLoadMatrixd(dv));
  TESTIT("glMatrixMode", glMatrixMode(GL_PROJECTION));
  TESTIT("glMultMatrixf", glMultMatrixf(fv));
  TESTIT("glMultMatrixd", glMultMatrixd(dv));
  TESTIT("glOrtho", glOrtho(-2.,2.,-2.,2.,-1.,1.));
  TESTIT("glPopMatrix", glPopMatrix());
  TESTIT("glPushMatrix", glPushMatrix());
  TESTIT("glRotated", glRotated(10.,1.,0.,0.));
  TESTIT("glRotatef", glRotatef(10.,1.,0.,0.));
  TESTIT("glScaled", glScaled(1.,2.,1.));
  TESTIT("glScalef", glScalef(1.,2.,1.));
  TESTIT("glTranslated", glTranslated(1.,2.,1.));
  TESTIT("glTranslatef", glTranslatef(1.,2.,1.));
  TESTIT("glViewport", glViewport(10,10,40,40));

#ifdef GL_VERSION_1_1
    /* 1.1 testing */
    if (IS_ONEONE()) {
	TESTIT("glTexSubImage1D", glTexSubImage1D(GL_TEXTURE_1D,0,0,1,GL_RGBA,GL_UNSIGNED_BYTE,stipple));
	TESTIT("glTexSubImage2D", glTexSubImage2D(GL_TEXTURE_2D,0,0,0,1,1,GL_RGBA,GL_UNSIGNED_BYTE,stipple));
	/*TESTIT("glArrayElement, glArrayElement(0));*/
	TESTIT("glDrawArrays", glDrawArrays(GL_POINTS, 1, 1));
	TESTIT("glGetPointerv", glGetPointerv(GL_VERTEX_ARRAY_POINTER, (void **) &conv));
	TESTIT("glAreTexturesResident", glAreTexturesResident(0, uiv, (GLboolean *) uiv));
	TESTIT("glBindTexture", glBindTexture(GL_TEXTURE_2D, 1));
	TESTIT("glDeleteTextures", glDeleteTextures(0, uiv));
	TESTIT("glGenTextures", glGenTextures(1, uiv));
	TESTIT("glIsTexture", glIsTexture(1));
	TESTIT("glPrioritizeTextures", glPrioritizeTextures(0, uiv, (GLclampf *) uiv));
	TESTIT("glCopyTexImage1D", glCopyTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 1, 1, 1, 0));
	TESTIT("glCopyTexImage2D", glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 1, 1, 0));
	TESTIT("glCopyTexSubImage1D", glCopyTexSubImage1D(GL_TEXTURE_1D, 0, 0, 0, 0, 1));
	TESTIT("glCopyTexSubImage2D", glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1,1));
	TESTIT("glColorPointer", glColorPointer(3, GL_FLOAT, 0, 0));
	TESTIT("glDisableClientState", glDisableClientState(GL_VERTEX_ARRAY));
	TESTIT("glDrawElements", glDrawElements(GL_POINTS, 0, GL_UNSIGNED_INT, 0));
	TESTIT("glEdgeFlagPointer", glEdgeFlagPointer(0, 0));
	TESTIT("glEnableClientState", glEnableClientState(GL_VERTEX_ARRAY));
	TESTIT("glIndexPointer", glIndexPointer(GL_FLOAT, 0, 0));
	TESTIT("glInterleavedArrays", glInterleavedArrays(GL_V2F, 0, 0));
	TESTIT("glNormalPointer", glNormalPointer(GL_FLOAT, 0, 0));
	TESTIT("glTexCoordPointer", glTexCoordPointer(2, GL_FLOAT, 0, 0));
	TESTIT("glVertexPointer", glVertexPointer(3, GL_FLOAT, 0, 0));
	TESTIT("glPolygonOffset", glPolygonOffset(1.,1.));
	/*TESTIT("glglIndexub", glIndexub(0));*/
	/*TESTIT("glglIndexubv", glIndexubv(ubzero));*/
	TESTIT("glPushClientAttrib", glPushClientAttrib(0));
	TESTIT("glPopClientAttrib", glPopClientAttrib());
    }
#endif
#define HAVE_EXT(ext) (strstr((const char *)ex, ext) != 0)
#define EX_TESTIT(ext,tag,x)	if(HAVE_EXT(ext)){TESTIT(tag,x);}

/*
 * Override the extension string. Execute a sample command 'cmd' to
 * see whether it generates an error. If not, all is well and fine
 */

#define FORCE_TESTIT(ext, tag, cmd, x)  {\
  cmd; \
  e = glGetError();\
  if (e == GL_NO_ERROR) { \
    TESTIT(tag, x);\
  }\
}

  glDeleteLists(1,1);
}

CLEANUP(bgnend) {
    ogLibSetDefaultTextures();
    glTexImage1D(GL_TEXTURE_1D, 0, 1, 0, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
}

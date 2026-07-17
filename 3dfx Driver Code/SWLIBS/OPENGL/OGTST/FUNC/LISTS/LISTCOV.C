#include "ogtst.h"

/*
 *  listcov.c - very simple test to sanity check display list coverage
 */

#define TESTIT(tag,x) \
        ogEnvLog(1, "Testing %s\n", tag); \
	glNewList(1,GL_COMPILE); x; glEndList();  glCallList(1); check_error(tag, _compile); \
	glNewList(1,GL_COMPILE_AND_EXECUTE); x; glEndList();  check_error(tag, _compile_and_execute);

#define CTESTIT(a,tag,x) \
	{ \
	  GLint v; \
	  glGetIntegerv(a,&v); \
          if (v) { \
	    ogEnvLog(1, "Testing %s\n", tag); \
	    glNewList(1,GL_COMPILE); x; glEndList();  glCallList(1); check_error(##tag, _compile); \
     	    glNewList(1,GL_COMPILE_AND_EXECUTE); x; glEndList();  check_error(##tag, _compile_and_execute); \
	    } \
	}
#define HAVE_EXT(ext) (strstr((const char *)ex, ext) != 0)

static void check_error(const char *tag, const char *style) {
    GLint e;

    ogEnvLog(3, "%-25s: %s\n", style, tag);
    do {
	if ((e = glGetError()) != GL_NO_ERROR)
	    ogEnvLog(OG_LFAIL, "%s generated error 0x%x\n", tag, e);
    } while (e != GL_NO_ERROR);
}

static const GLubyte *ex;

/*ARGSUSED*/
TESTMOD(listcov) {
    GLdouble dv[16];
    GLfloat fv[16];
    GLfloat ffv[129];
    GLint iv[16];
    GLuint uiv[4];
    GLshort sv[4];
    GLushort usv[4];
    GLubyte stipple[128];
    GLbyte bzero[] = { 0, 0, 0, 0};
    GLshort szero[] = { 0, 0, 0, 0};
    GLint izero[] = { 0, 0, 0, 0 };
    GLubyte ubzero[] = { 0, 0, 0, 0};
    GLushort uszero[] = { 0, 0, 0, 0};
    GLuint uizero[] = { 0, 0, 0, 0};
    GLfloat fzero[] = { 0., 0., 0., 0. };
    GLdouble dzero[] = { 0., 0., 0., 0. };
    GLfloat mf[] = {0.1, 0.2, 0.3, 2.0};
    GLint mi[] = {10, 20, 30, 200};
    GLfloat fsp[] = {0.0, 0.0, -1.0, 0.5, -2.0, 1.0, -3.0, 1.5};
    GLubyte conv[128], row[128], col[128];
    static const char *_compile = "compile";
    static const char *_compile_and_execute = "compile and execute";
    int hwtype = ogEnvQuery(OG_HW);
    GLint e, i;

    for (i = 0; i < 128; i++) stipple[i] = 0xff;

    TESTIT("glListBase", glListBase(0));
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
    TESTIT("glFogfv", glFogfv(GL_FOG_START,fzero));
    TESTIT("glFogi", glFogi(GL_FOG_MODE,GL_LINEAR));
    TESTIT("glFogiv", glFogiv(GL_FOG_START,izero));
    TESTIT("glFrontFace", glFrontFace(GL_CCW));
    TESTIT("glHint", glHint(GL_FOG_HINT,GL_NICEST));
    TESTIT("glLightf", glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,0.));
    TESTIT("glLightfv", glLightfv(GL_LIGHT0,GL_SPOT_EXPONENT,fzero));
    TESTIT("glLighti", glLighti(GL_LIGHT0,GL_SPOT_EXPONENT,0));
    TESTIT("glLightiv", glLightiv(GL_LIGHT0,GL_SPOT_EXPONENT,izero));
    TESTIT("glLightModelf", glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,1.));
    TESTIT("glLightModelfv", glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE,fv));
    TESTIT("glLightModeli", glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1));
    TESTIT("glLightModeliv", glLightModeliv(GL_LIGHT_MODEL_TWO_SIDE,iv));
    TESTIT("glMateriali", glMateriali(GL_FRONT, GL_SHININESS, 5));
    TESTIT("glMaterialf", glMaterialf(GL_FRONT, GL_SHININESS, 5.0));
    TESTIT("glMaterialfv", glMaterialfv(GL_FRONT, GL_AMBIENT, mf));
    TESTIT("glMaterialiv", glMaterialiv(GL_FRONT, GL_AMBIENT, mi));

    TESTIT("glLineStipple", glLineStipple(5,0xbabe));
    TESTIT("glLineWidth", glLineWidth(2.));
    TESTIT("glPointSize", glPointSize(2.));
    TESTIT("glPolygonMode", glPolygonMode(GL_FRONT,GL_LINE));
    TESTIT("glPolygonStipple", glPolygonStipple(stipple));
    TESTIT("glScissor", glScissor(10,10,20,20));
    TESTIT("glShadeModel", glShadeModel(GL_FLAT));
    TESTIT("glTexParameterf", glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST));
    TESTIT("glTexParameterfv", glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,fzero));
    TESTIT("glTexParameteri", glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST));
    TESTIT("glTexParameteriv", glTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,izero));

    CTESTIT(GL_RGBA_MODE,"glTexImage1D", glTexImage1D(GL_TEXTURE_1D,0,4,1,0,GL_RGBA,GL_UNSIGNED_BYTE,stipple)); /*XXXblythe*/
    CTESTIT(GL_RGBA_MODE,"glTexImage2D", glTexImage2D(GL_TEXTURE_2D,0,4,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,stipple)); /*XXXblythe*/

    TESTIT("glTexEnvf", glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_BLEND));
    TESTIT("glTexEnvfv", glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,fzero));
    TESTIT("glTexEnvi", glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_BLEND));
    TESTIT("glTexEnviv", glTexEnviv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,izero));
    TESTIT("glTexGend", glTexGend(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR));
    TESTIT("glTexGendv", glTexGendv(GL_S, GL_OBJECT_PLANE, dzero));
    TESTIT("glTexGenf", glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR));
    TESTIT("glTexGenfv", glTexGenfv(GL_S, GL_OBJECT_PLANE, fzero));
    TESTIT("glTexGeni", glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR));
    TESTIT("glTexGeniv", glTexGeniv(GL_S, GL_OBJECT_PLANE, izero));
    TESTIT("glInitNames", glInitNames());
    TESTIT("glLoadName", glLoadName(0xbabe));
    TESTIT("glPassThrough", glPassThrough(0xbabe));
    TESTIT("glPushName", glPushName(0xbabe));	/*invert order to avoid errors*/
    TESTIT("glPopName", glPopName());
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
    CTESTIT(GL_ACCUM_RED_BITS,"glAccum", glAccum(GL_LOAD,0.5));
    TESTIT("glDisable", glDisable(GL_BLEND));
    TESTIT("glEnable", glEnable(GL_BLEND));
    TESTIT("glPushAttrib", glPushAttrib(GL_CURRENT_BIT));	/*invert order to avoid errors*/
    TESTIT("glPopAttrib", glPopAttrib());
    TESTIT("glMap1d", glMap1d(GL_MAP1_TEXTURE_COORD_1,1,2,2,2,dv));
    TESTIT("glMap1f", glMap1f(GL_MAP1_TEXTURE_COORD_1,1,2,2,2,fv));
    TESTIT("glMap2d", glMap2d(GL_MAP2_TEXTURE_COORD_1,1,2,4,2,1,2,2,2,dv));
    TESTIT("glMap2f", glMap2f(GL_MAP2_TEXTURE_COORD_1,1,2,4,2,1,2,2,2,fv));
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

    TESTIT("glPixelMapfv", glPixelMapfv(GL_PIXEL_MAP_I_TO_G,4,fv));
    TESTIT("glPixelMapuiv", glPixelMapuiv(GL_PIXEL_MAP_I_TO_G,4,uiv));
    TESTIT("glPixelMapusv", glPixelMapusv(GL_PIXEL_MAP_I_TO_G,4,usv));
    TESTIT("glReadBuffer", glReadBuffer(GL_FRONT));
    TESTIT("glCopyPixels", glCopyPixels(10,10,10,10,GL_COLOR));

#if 0
    /*XXXblythe fix me*/
    TESTIT("glDrawPixels", glDrawPixels(2,2,GL_BITMAP,GL_COLOR_INDEX,stipple));
#endif

    TESTIT("glDepthRange", glDepthRange(0.25,0.75));
    TESTIT("glFrustum", glFrustum(-2.,2.,-2.,2.,0.1,1.));
    TESTIT("glLoadIdentity", glLoadIdentity());
    TESTIT("glLoadMatrixf", glLoadMatrixf(fv));
    TESTIT("glLoadMatrixd", glLoadMatrixd(dv));
    TESTIT("glMatrixMode", glMatrixMode(GL_MODELVIEW));
    TESTIT("glMultMatrixf", glMultMatrixf(fv));
    TESTIT("glMultMatrixd", glMultMatrixd(dv));
    TESTIT("glOrtho", glOrtho(-2.,2.,-2.,2.,-1.,1.));
    TESTIT("glPushMatrix", glPushMatrix());	/*invert order to avoid errors*/
    TESTIT("glPopMatrix", glPopMatrix());
    TESTIT("glRotated", glRotated(10.,1.,0.,0.));
    TESTIT("glRotatef", glRotatef(10.,1.,0.,0.));
    TESTIT("glScaled", glScaled(1.,2.,1.));
    TESTIT("glScalef", glScalef(1.,2.,1.));
    TESTIT("glTranslated", glTranslated(1.,2.,1.));
    TESTIT("glTranslatef", glTranslatef(1.,2.,1.));
    TESTIT("glViewport", glViewport(10,10,40,40));

    TESTIT("glVertex2d", glVertex2d(0.,.0));
    TESTIT("glVertex2dv", glVertex2dv(dzero));
    TESTIT("glVertex2f", glVertex2f(0.,0.));
    TESTIT("glVertex2fv", glVertex2fv(fzero));
    TESTIT("glVertex2i", glVertex2i(0,0));
    TESTIT("glVertex2iv", glVertex2iv(izero));
    TESTIT("glVertex2s", glVertex2s(0,0));
    TESTIT("glVertex2sv", glVertex2sv(szero));
    TESTIT("glVertex3d", glVertex3d(0.,0.,0.));
    TESTIT("glVertex3dv", glVertex3dv(dzero));
    TESTIT("glVertex3f", glVertex3f(0.,0.,0.));
    TESTIT("glVertex3fv", glVertex3fv(fzero));
    TESTIT("glVertex3i", glVertex3i(0,0,0));
    TESTIT("glVertex3iv", glVertex3iv(izero));
    TESTIT("glVertex3s", glVertex3s(0,0,0));
    TESTIT("glVertex3sv", glVertex3sv(szero));
    TESTIT("glVertex4d", glVertex4d(0.,0.,0.,0.));
    TESTIT("glVertex4dv", glVertex4dv(dzero));
    TESTIT("glVertex4f", glVertex4f(0.,0.,0.,0.));
    TESTIT("glVertex4fv", glVertex4fv(fzero));
    TESTIT("glVertex4i", glVertex4i(0,0,0,0));
    TESTIT("glVertex4iv", glVertex4iv(izero));
    TESTIT("glVertex4s", glVertex4s(0,0,0,0));
    TESTIT("glVertex4sv", glVertex4sv(szero));

    TESTIT("glNormal3b", glNormal3b(0,0,0));
    TESTIT("glNormal3bv", glNormal3bv(bzero));
    TESTIT("glNormal3d", glNormal3d(0.,0.,0.));
    TESTIT("glNormal3dv", glNormal3dv(dzero));
    TESTIT("glNormal3f", glNormal3f(0.,0.,0.));
    TESTIT("glNormal3fv", glNormal3fv(fzero));
    TESTIT("glNormal3i", glNormal3i(0,0,0));
    TESTIT("glNormal3iv", glNormal3iv(izero));
    TESTIT("glNormal3s", glNormal3s(0,0,0));
    TESTIT("glNormal3sv", glNormal3sv(szero));

    TESTIT("glColor3b", glColor3b(0,0,0));
    TESTIT("glColor3bv", glColor3bv(bzero));
    TESTIT("glColor3d", glColor3d(0.,0.,0.));
    TESTIT("glColor3dv", glColor3dv(dzero));
    TESTIT("glColor3f", glColor3f(0.,0.,0.));
    TESTIT("glColor3fv", glColor3fv(fzero));
    TESTIT("glColor3i", glColor3i(0,0,0));
    TESTIT("glColor3iv", glColor3iv(izero));
    TESTIT("glColor3s", glColor3s(0,0,0));
    TESTIT("glColor3sv", glColor3sv(szero));
    TESTIT("glColor3ub", glColor3ub(0,0,0));
    TESTIT("glColor3ubv", glColor3ubv(ubzero));
    TESTIT("glColor3ui", glColor3ui(0,0,0));
    TESTIT("glColor3uiv", glColor3uiv(uizero));
    TESTIT("glColor3us", glColor3us(0,0,0));
    TESTIT("glColor3usv", glColor3usv(uszero));
    TESTIT("glColor4b", glColor4b(0,0,0,0));
    TESTIT("glColor4bv", glColor4bv(bzero));
    TESTIT("glColor4d", glColor4d(0.,0.,0.,0.));
    TESTIT("glColor4dv", glColor4dv(dzero));
    TESTIT("glColor4f", glColor4f(0.,0.,0.,0.));
    TESTIT("glColor4fv", glColor4fv(fzero));
    TESTIT("glColor4i", glColor4i(0,0,0,0));
    TESTIT("glColor4iv", glColor4iv(izero));
    TESTIT("glColor4s", glColor4s(0,0,0,0));
    TESTIT("glColor4sv", glColor4sv(szero));
    TESTIT("glColor4ub", glColor4ub(0,0,0,0));
    TESTIT("glColor4ubv", glColor4ubv(ubzero));
    TESTIT("glColor4ui", glColor4ui(0,0,0,0));
    TESTIT("glColor4uiv", glColor4uiv(uizero));
    TESTIT("glColor4us", glColor4us(0,0,0,0));
    TESTIT("glColor4usv", glColor4usv(uszero));

    TESTIT("glIndexd", glIndexd(0.));
    TESTIT("glIndexdv", glIndexdv(dzero));
    TESTIT("glIndexf", glIndexf(0.));
    TESTIT("glIndexfv", glIndexfv(fzero));
    TESTIT("glIndexi", glIndexi(0));
    TESTIT("glIndexiv", glIndexiv(izero));
    TESTIT("glIndexs", glIndexs(0));
    TESTIT("glIndexsv", glIndexsv(szero));

    TESTIT("glTexCoord1d", glTexCoord1d(0.));
    TESTIT("glTexCoord1dv", glTexCoord1dv(dzero));
    TESTIT("glTexCoord1f", glTexCoord1f(0.));
    TESTIT("glTexCoord1fv", glTexCoord1fv(fzero));
    TESTIT("glTexCoord1i", glTexCoord1i(0));
    TESTIT("glTexCoord1iv", glTexCoord1iv(izero));
    TESTIT("glTexCoord1s", glTexCoord1s(0));
    TESTIT("glTexCoord1sv", glTexCoord1sv(szero));
    TESTIT("glTexCoord2d", glTexCoord2d(0.,0.));
    TESTIT("glTexCoord2dv", glTexCoord2dv(dzero));
    TESTIT("glTexCoord2f", glTexCoord2f(0.,0.));
    TESTIT("glTexCoord2fv", glTexCoord2fv(fzero));
    TESTIT("glTexCoord2i", glTexCoord2i(0,0));
    TESTIT("glTexCoord2iv", glTexCoord2iv(izero));
    TESTIT("glTexCoord2s", glTexCoord2s(0,0));
    TESTIT("glTexCoord2sv", glTexCoord2sv(szero));
    TESTIT("glTexCoord3d", glTexCoord3d(0.,0.,0.));
    TESTIT("glTexCoord3dv", glTexCoord3dv(dzero));
    TESTIT("glTexCoord3f", glTexCoord3f(0.,0.,0.));
    TESTIT("glTexCoord3fv", glTexCoord3fv(fzero));
    TESTIT("glTexCoord3i", glTexCoord3i(0,0,0));
    TESTIT("glTexCoord3iv", glTexCoord3iv(izero));
    TESTIT("glTexCoord3s", glTexCoord3s(0,0,0));
    TESTIT("glTexCoord3sv", glTexCoord3sv(szero));
    TESTIT("glTexCoord4d", glTexCoord4d(0.,0.,0.,0.));
    TESTIT("glTexCoord4dv", glTexCoord4dv(dzero));
    TESTIT("glTexCoord4f", glTexCoord4f(0.,0.,0.,0.));
    TESTIT("glTexCoord4fv", glTexCoord4fv(fzero));
    TESTIT("glTexCoord4i", glTexCoord4i(0.,0.,0.,0.));
    TESTIT("glTexCoord4iv", glTexCoord4iv(izero));
    TESTIT("glTexCoord4s", glTexCoord4s(0.,0.,0.,0.));
    TESTIT("glTexCoord4sv", glTexCoord4sv(szero));


#ifdef GL_VERSION_1_1
    /* 1.1 testing */
    if (IS_ONEONE()) {
	CTESTIT(GL_RGBA_MODE,"glTexSubImage1D", glTexSubImage1D(GL_TEXTURE_1D,0,0,1,GL_RGBA,GL_UNSIGNED_BYTE,stipple));
	CTESTIT(GL_RGBA_MODE,"glTexSubImage2D", glTexSubImage2D(GL_TEXTURE_2D,0,0,0,1,1,GL_RGBA,GL_UNSIGNED_BYTE,stipple));
#if 0
glArrayElement
#endif
	TESTIT("glDrawArrays", glDrawArrays(GL_POINTS, 1, 1));
	TESTIT("glGetPointerv", glGetPointerv(GL_VERTEX_ARRAY_POINTER, (void **) &conv));
	TESTIT("glAreTexturesResident", glAreTexturesResident(0, uiv, (GLboolean *) uiv));
	TESTIT("glBindTexture", glBindTexture(GL_TEXTURE_2D, 1));
	TESTIT("glDeleteTextures", glDeleteTextures(0, uiv));
	TESTIT("glGenTextures", glGenTextures(1, uiv));
	TESTIT("glIsTexture", glIsTexture(1));
	TESTIT("glPrioritizeTextures", glPrioritizeTextures(0, uiv, (GLclampf *) uiv));
	CTESTIT(GL_RGBA_MODE,"glCopyTexImage1D", glCopyTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 1, 1, 1, 0));
	CTESTIT(GL_RGBA_MODE,"glCopyTexImage2D", glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 1, 1, 0));
	CTESTIT(GL_RGBA_MODE,"glCopyTexSubImage1D", glCopyTexSubImage1D(GL_TEXTURE_1D, 0, 0, 0, 0, 1));
	CTESTIT(GL_RGBA_MODE,"glCopyTexSubImage2D", glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1,1));
#if 0
ColorPointer
DisableClientState
DrawElements
EdgeFlagPointer
EnableClientState
IndexPointer
InterleavedArrays
NormalPointer
TexCoordPointer
VertexPointer
#endif
	TESTIT("glPolygonOffset", glPolygonOffset(1.,1.));
	TESTIT("glIndexub", glIndexub(0));
	TESTIT("glIndexubv", glIndexubv(ubzero));
#if 0
PopClientAttrib
PushClientAttrib
#endif
    }
#endif

    ex = glGetString(GL_EXTENSIONS);

#define EX_TESTIT(ext,tag,x)	if(HAVE_EXT(ext)){TESTIT(tag,x);}
#define EX_CTESTIT(ext,a,tag,x)	if(HAVE_EXT(ext)){CTESTIT(a,tag,x);}

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

CLEANUP(listcov) {
  GLfloat envcolor[] = { 0, 0, 0, 0 };
  GLfloat fone[] = { 1., 1., 1., 1. };
  GLfloat fident[] = { 1., 0., 0., 0. };
  GLfloat fz = 0.;
  const GLubyte *ex = glGetString(GL_EXTENSIONS);
  GLint n, mask, rgb;

  glGetIntegerv(GL_RGBA_MODE,&rgb);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glPixelStorei(GL_PACK_ALIGNMENT, 4);

  /* This texure stuff seems necessary insplite of later call to */
  /* ogLibSetDefaultTextures(). State is not cleanup up properly if */
  /* this code not present */

  if (rgb)
      glTexImage1D(GL_TEXTURE_1D,0,0,0,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		  GL_NEAREST_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		  GL_LINEAR);
  glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,envcolor);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envcolor);
  glTexCoord2f(0, 0);

  /* Viewport and scissoring */
  glViewport(0, 0, ogEnvQuery(OG_XWSIZE),ogEnvQuery(OG_YWSIZE));
  glScissor(0, 0, ogEnvQuery(OG_XWSIZE),ogEnvQuery(OG_YWSIZE));

  glShadeModel(GL_SMOOTH);
  glNormal3f(0., 0., 1.);
  
  glDepthFunc(GL_LESS);
  glDepthRange(0., 1.);
  glDisable(GL_DEPTH_TEST);

  glGetIntegerv(GL_STENCIL_BITS, &n);
  mask = (1<<n) - 1;
  glStencilMask(mask);
  glStencilFunc(GL_ALWAYS, 0, mask);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  glDisable(GL_STENCIL_TEST);

  glPixelZoom(1., 1.);

  /****/

  glPointSize(1);
  glLineWidth(1);
  ogLibSetDefaultFog();

  ogLibSetDefaultBlend();
  ogLibSetDefaultMatrices();
  
  ogLibSetDefaultMatrices();
  ogLibSetDefaultBuffers();
  ogLibSetDefaultColors();
  ogLibSetDefaultClears();
  ogLibSetDefaultLight();
  glTexGenfv(GL_S, GL_OBJECT_PLANE, fident);
  ogLibSetDefaultTextures();
  ogLibSetDefaultEvaluators();
  ogLibSetDefaultFog();
  ogLibSetDefaultRasterPos();

  glAlphaFunc(GL_ALWAYS, 0);
  glClearColor(0,0,0,0);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  glIndexMask(0xff);

  glCullFace(GL_BACK);

  glPolygonMode(GL_FRONT, GL_FILL);
  glPolygonMode(GL_BACK, GL_FILL);

  glPixelTransferf(GL_RED_BIAS, 0.0);
  {
    GLdouble clip[4] = {0., 0., 0., 0.};
    glClipPlane(GL_CLIP_PLANE0, clip);
  }
  glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 1, &fz);
  glDisable(GL_POLYGON_STIPPLE);
#ifdef GL_VERSION_1_1
    /* 1.1 testing */
    if (IS_ONEONE()) {
	glPolygonOffset(0., 0.);
    }
#endif

  glDisable(GL_TEXTURE_2D);

}

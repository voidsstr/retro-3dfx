#include "ogtst.h"

/*
 *  errors.c - very simple test to sanity check error handling
 *  in immediate mode, list compilation, and list execution.
 *
 */

#define _TESTIT(CMD, ERR) do { \
    ogEnvLog(1, "Testing %s\n", #CMD); \
    CMD;  \
    check_error(#CMD, ERR, _immediate); \
    glNewList(1,GL_COMPILE); \
    CMD; \
    glEndList(); \
    glCallList(1); \
    check_error(#CMD, ERR, _compile); \
    glNewList(1,GL_COMPILE_AND_EXECUTE); \
    CMD; \
    glEndList(); \
    check_error(#CMD, ERR, _compile_and_execute); \
} while (0)

#define TESTITE(CMD) _TESTIT(CMD, GL_INVALID_ENUM)
#define TESTITO(CMD) _TESTIT(CMD, GL_INVALID_OPERATION)
#define TESTITV(CMD) _TESTIT(CMD, GL_INVALID_VALUE)

#define CTESTITE(ATTR, CMD) do { \
    GLint v; \
    glGetIntegerv(ATTR,&v); \
    if (v) \
        TESTITE(CMD); \
} while (0)

#define CTESTITV(ATTR, CMD) do { \
    GLint v; \
    glGetIntegerv(ATTR, &v); \
    if (v) \
        TESTITV(CMD); \
} while (0)

#define CTESTITO(ATTR, CMD) do { \
    GLint v; \
    glGetIntegerv(ATTR, &v); \
    if (v) \
        TESTITO(CMD); \
} while (0)

#define HAVE_EXT(ext) (strstr((const char *)ex, ext) != 0)

#define EX_TESTITE(ext,x) do { \
    if (HAVE_EXT(ext)) TESTITE(x); } while (0)
#define EX_TESTITV(ext,x) do { \
    if (HAVE_EXT(ext)) TESTITV(x); } while (0)
#define EX_TESTITO(ext,x) do { \
    if (HAVE_EXT(ext)) TESTITO(x); } while (0)
#define EX_CTESTITE(ext, c, x) do { \
    if (HAVE_EXT(ext)) CTESTITE(c, x); } while (0)
#define EX_CTESTITV(ext, c, x) do { \
    if (HAVE_EXT(ext)) CTESTITV(c, x); } while (0)
#define EX_CTESTITO(ext, c, x) do { \
    if (HAVE_EXT(ext)) CTESTITO(c, x); } while (0)
/*
 * Override the extension string. Execute a sample command 'cmd' to
 * see whether it generates an error. If not, all is well and fine
 */
#define FORCE_TESTITE(ext, cmd, x)  do {\
  cmd; \
  e = glGetError();\
  if (e == GL_NO_ERROR) { \
    TESTITE(x);\
  }\
} while (0)
#define FORCE_TESTITV(ext, cmd, x) do {\
  cmd; \
  e = glGetError();\
  if (e == GL_NO_ERROR) { \
    TESTITV(x);\
  }\
} while (0)

#define CALL1(macro, ext, param, func, x1) do {\
    macro(ext, param, func x1); \
} while (0)
#define CALL2(macro, ext, param, func, x1, x2) do { \
    macro(ext, param, func x1); \
    CALL1(macro, ext, param, func, x2); \
} while (0)
#define CALL3(macro, ext, param, func, x1, x2, x3) do { \
    macro(ext, param, func x1); \
    CALL2(macro, ext, param, func, x2, x3); \
} while (0)
#define CALL4(macro, ext, param, func, x1, x2, x3, x4) do { \
    macro(ext, param, func x1); \
    CALL3(macro, ext, param, func, x2, x3, x4); \
} while (0)
#define CALL5(macro, ext, param, func, x1, x2, x3, x4, x5) do { \
    macro(ext, param, func x1); \
    CALL4(macro, ext, param, func, x2, x3, x4, x5); \
} while (0)
#define CALL6(macro, ext, param, func, x1, x2, x3, x4, x5, x6) do { \
    macro(ext, param, func x1); \
    CALL5(macro, ext, param, func, x2, x3, x4, x5, x6); \
} while (0)
#define CALL7(macro, ext, param, func, x1, x2, x3, x4, x5, x6, x7) do { \
    macro(ext, param, func x1); \
    CALL6(macro, ext, param, func, x2, x3, x4, x5, x6, x7); \
} while (0)

/* Slurp the extra parameter for CALL?() */
#define SLURP_EX_TESTITE(ext, param, x) EX_TESTITE(ext, x)
#define SLURP_EX_TESTITV(ext, param, x) EX_TESTITV(ext, x)
#define SLURP_EX_TESTITO(ext, param, x) EX_TESTITO(ext, x)

static void check_error(const char *tag, GLenum error, const char *style) {
    GLint e;

    ogEnvLog(3, "%-25s: %s\n", style, tag);
    if ((e = glGetError()) != error) {
	ogEnvLog(OG_LFAIL, "%s in %s mode got error 0x%x, expected 0x%x\n",
                 tag, style, e, error);
	while (e != GL_NO_ERROR) {
	    if ((e = glGetError()) != GL_NO_ERROR)
		ogEnvLog(OG_LFAIL, "%s also received error 0x%x\n", tag, e);
	}
    }
}

static const GLubyte *ex;

/*ARGSUSED*/
TESTMOD(errors) {
    GLdouble dv[16];
    GLfloat fv[16];
    GLfloat ffv[129];
    GLint iv[16];
    GLuint uiv[4];
    GLushort usv[4];
    GLubyte stipple[128];
    GLint izero[] = { 0, 0, 0, 0 };
    GLfloat fzero[] = { 0., 0., 0., 0. };
    GLdouble dzero[] = { 0., 0., 0., 0. };
    GLfloat fsp[] = {0.0, 0.0, -1.0, 0.5, -2.0, 1.0, -3.0, 1.5};
    GLint iminus[] = { -1, -1, -1, -1 };
    GLfloat fminus[] = { -1, -1, -1, -1 };
    GLdouble dminus[] = { -1, -1, -1, -1 };
    static const char *_compile = "compile";
    static const char *_compile_and_execute = "compile and execute";
    static const char *_immediate = "immediate";
    GLint e;
    GLubyte conv[128];
    GLubyte row[128], col[128];
    int hwtype = ogEnvQuery(OG_HW);
    int aux_buffers = ogEnvCurVisualInfo(GLX_AUX_BUFFERS);

    TESTITE(glBegin(-1));       /* bad enum */
    TESTITV(glBitmap(-1,0,0.,0.,10,10,NULL));	/* negative width */
    TESTITV(glBitmap(0,-1,0.,0.,10,10,NULL));	/* negative height */

    TESTITE(glClipPlane(-1,dv));		/* bad clip plan enumerant */
    TESTITE(glClipPlane(GL_CLIP_PLANE0+GL_MAX_CLIP_PLANES,dv));	/* bad clip plan enumerant */

    TESTITE(glColorMaterial(-1,GL_AMBIENT));	/* bad face enumerant */
    TESTITE(glColorMaterial(GL_FRONT,-1));	/* bad mode enumerant */

    TESTITE(glCullFace(-1));			/* bad face enumerant */

    TESTITE(glFogf(-1,GL_LINEAR));		/* bad pname */
    TESTITE(glFogf(GL_FOG_MODE,-1));		/* bad mode */

    TESTITE(glFogi(-1,GL_LINEAR));		/* bad pname */
    TESTITE(glFogi(GL_FOG_MODE,-1));		/* bad mode */

    TESTITE(glFrontFace(-1));			/* bad direction */

    TESTITE(glHint(-1,GL_NICEST));		/* bad target */
    TESTITE(glHint(GL_FOG_HINT,-1));		/* bad mode */

    TESTITE(glLightf(-1,GL_SPOT_EXPONENT,0.));			/* bad light */
    TESTITE(glLightf(GL_LIGHT0+GL_MAX_LIGHTS,GL_AMBIENT,0.));	/* bad light */
    TESTITE(glLightf(GL_LIGHT0,-1,0.));				/* bad pname */
    TESTITV(glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,-1));		/* bad value */
    TESTITV(glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,-1));		/* bad value */
    TESTITV(glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,-1));	/* bad value */
    TESTITV(glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,-1));	/* bad value */
    TESTITV(glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,-1));	/* bad value */

    TESTITV(glLightfv(GL_LIGHT0,GL_SPOT_EXPONENT,fminus));	/* bad value */

    TESTITE(glLighti(-1,GL_SPOT_EXPONENT,0));			/* bad light */
    TESTITE(glLighti(GL_LIGHT0+GL_MAX_LIGHTS,GL_AMBIENT,0));	/* bad light */
    TESTITE(glLighti(GL_LIGHT0,-1,0));				/* bad pname */
    TESTITV(glLighti(GL_LIGHT0,GL_SPOT_EXPONENT,-1));		/* bad value */
    TESTITV(glLighti(GL_LIGHT0,GL_SPOT_CUTOFF,-1));		/* bad value */
    TESTITV(glLighti(GL_LIGHT0,GL_CONSTANT_ATTENUATION,-1));	/* bad value */
    TESTITV(glLighti(GL_LIGHT0,GL_LINEAR_ATTENUATION,-1));	/* bad value */
    TESTITV(glLighti(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,-1));	/* bad value */

    TESTITV(glLightiv(GL_LIGHT0,GL_SPOT_EXPONENT,iminus));	/* bad value */

    TESTITE(glLightModelf(-1,1.));				/* bad pname */
    TESTITE(glLightModelfv(-1,fv));				/* bad pname */
    TESTITE(glLightModeli(-1,1));				/* bad pname */
    TESTITE(glLightModeliv(-1,iv));				/* bad pname */

    TESTITV(glLineWidth(0.));					/* bad width */
    TESTITV(glPointSize(0.));					/* bad size */

    TESTITE(glPolygonMode(-1,GL_LINE));			/* bad face */
    TESTITE(glPolygonMode(GL_FRONT,-1));			/* bad mode */

    TESTITV(glScissor(10,10,-1,20));				/* bad width */
    TESTITV(glScissor(10,10,20,-1));				/* bad height */

    TESTITE(glShadeModel(-1));					/* bad pname */

    /*XXXblythe more stuff here*/
    TESTITE(glTexParameterf(-1,GL_TEXTURE_MAG_FILTER,GL_NEAREST));	/* bad target */
    TESTITE(glTexParameterf(GL_TEXTURE_2D,-1,GL_NEAREST));		/* bad pname */
    TESTITE(glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,-1));	/* bad filter */

    TESTITE(glTexParameterfv(-1,GL_TEXTURE_BORDER_COLOR,fzero));
    TESTITE(glTexParameterfv(GL_TEXTURE_2D,-1,fzero));

    TESTITE(glTexParameteri(-1,GL_TEXTURE_MAG_FILTER,GL_NEAREST));
    TESTITE(glTexParameteri(GL_TEXTURE_2D,-1,GL_NEAREST));
    TESTITE(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,-1));

    TESTITE(glTexParameteriv(-1,GL_TEXTURE_BORDER_COLOR,izero));
    TESTITE(glTexParameteriv(GL_TEXTURE_2D,-1,izero));

    /*XXXblythe more test cases*/
    CTESTITE(GL_RGBA_MODE,glTexImage1D(-1,0,4,1,0,GL_RGBA,GL_UNSIGNED_BYTE,stipple));
    CTESTITE(GL_RGBA_MODE,glTexImage1D(GL_TEXTURE_1D,0,4,1,0,-1,
                                       GL_UNSIGNED_BYTE,stipple));
    CTESTITE(GL_RGBA_MODE,glTexImage1D(GL_TEXTURE_1D,0,4,1,0,GL_RGBA,-1,stipple));

    CTESTITE(GL_RGBA_MODE,glTexImage2D(-1,0,4,1,1,0,GL_RGBA,
                                       GL_UNSIGNED_BYTE,stipple));
    CTESTITE(GL_RGBA_MODE,glTexImage2D(GL_TEXTURE_2D,0,4,1,1,0,-1,
                                       GL_UNSIGNED_BYTE,stipple));
    CTESTITE(GL_RGBA_MODE,glTexImage2D(GL_TEXTURE_2D,0,4,1,1,0,
                                       GL_RGBA,-1,stipple));

    TESTITE(glTexEnvf(-1,GL_TEXTURE_ENV_MODE,GL_BLEND));
    TESTITE(glTexEnvf(GL_TEXTURE_ENV,-1,GL_BLEND));
    TESTITE(glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,-1));

    TESTITE(glTexEnvfv(-1,GL_TEXTURE_ENV_COLOR,fzero));
    TESTITE(glTexEnvfv(GL_TEXTURE_ENV,-1,fzero));
    TESTITE(glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,fminus));

    TESTITE(glTexEnvi(-1,GL_TEXTURE_ENV_MODE,GL_BLEND));
    TESTITE(glTexEnvi(GL_TEXTURE_ENV,-1,GL_BLEND));
    TESTITE(glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,-1));

    TESTITE(glTexEnviv(-1,GL_TEXTURE_ENV_COLOR,izero));
    TESTITE(glTexEnviv(GL_TEXTURE_ENV,-1,izero));
    TESTITE(glTexEnviv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,iminus));

    TESTITE(glTexGend(-1, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR));
    TESTITE(glTexGend(GL_S, -1, GL_OBJECT_LINEAR));
    TESTITE(glTexGend(GL_S, GL_TEXTURE_GEN_MODE, -1));

    TESTITE(glTexGendv(-1, GL_OBJECT_PLANE, dzero));
    TESTITE(glTexGendv(GL_S, -1, dzero));
    TESTITE(glTexGendv(GL_S, GL_TEXTURE_GEN_MODE, dminus));

    TESTITE(glTexGenf(-1, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR));
    TESTITE(glTexGenf(GL_S, -1, GL_OBJECT_LINEAR));
    TESTITE(glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, -1));

    TESTITE(glTexGenfv(-1, GL_OBJECT_PLANE, fzero));
    TESTITE(glTexGenfv(GL_S, -1, fzero));
    TESTITE(glTexGenfv(GL_S, GL_TEXTURE_GEN_MODE, fminus));

    TESTITE(glTexGeni(-1, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR));
    TESTITE(glTexGeni(GL_S, -1, GL_OBJECT_LINEAR));
    TESTITE(glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, -1));

    TESTITE(glTexGeniv(-1, GL_OBJECT_PLANE, izero));
    TESTITE(glTexGeniv(GL_S, -1, izero));
    TESTITE(glTexGeniv(GL_S, GL_TEXTURE_GEN_MODE, iminus));

    TESTITE(glDrawBuffer(GL_ONE));		/* bad pname */
    if (aux_buffers == 0)
	TESTITO(glDrawBuffer(GL_AUX0)); /* bad operation */
    else
	TESTITO(glDrawBuffer(GL_AUX0 + aux_buffers)); /* bad operation */
    TESTITO(glDrawBuffer(GL_AUX0+ 299)); /* bad operation */
    TESTITV(glClear(-1));			/* extra bits */
    if (ogEnvCurVisualInfo(GLX_ACCUM_RED_SIZE) != 0) {
        TESTITE(glAccum(-1,0.5));/* bad pname */
    } else {
        TESTITO(glAccum(GL_ACCUM,0.5));/* invalid operation */
    }
    TESTITE(glDisable(-1));			/* bad pname */
    TESTITE(glEnable(-1));			/* bad pname */

    TESTITE(glMap1d(-1,1,2,2,2,dv));				/* bad map */
    TESTITV(glMap1d(GL_MAP1_TEXTURE_COORD_1,1,1,2,2,dv));	/* u1 == u2 */
    TESTITV(glMap1d(GL_MAP1_TEXTURE_COORD_1,1,2,0,2,dv));	/* bad stride */
    TESTITV(glMap1d(GL_MAP1_TEXTURE_COORD_1,1,2,2,0,dv));	/* bad order */
    TESTITV(glMap1d(GL_MAP1_TEXTURE_COORD_1,1,2,0,99,dv));	/* bad order */

    TESTITE(glMap1f(-1,1,2,2,2,fv));				/* bad map */
    TESTITV(glMap1f(GL_MAP1_TEXTURE_COORD_1,1,1,2,2,fv));	/* u1 == u2 */
    TESTITV(glMap1f(GL_MAP1_TEXTURE_COORD_1,1,2,0,2,fv));	/* bad stride */
    TESTITV(glMap1f(GL_MAP1_TEXTURE_COORD_1,1,2,2,0,fv));	/* bad order */
    TESTITV(glMap1f(GL_MAP1_TEXTURE_COORD_1,1,2,2,99,fv));	/* bad order */

    TESTITE(glMap2d(-1,1,2,4,2,1,2,2,2,dv));				/* bad map */
    TESTITV(glMap2d(GL_MAP2_TEXTURE_COORD_1,2,2,4,2,1,2,2,2,dv));	/* u1 = u2 */
    TESTITV(glMap2d(GL_MAP2_TEXTURE_COORD_1,1,2,0,2,1,2,2,2,dv));	/* bad u stride */
    TESTITV(glMap2d(GL_MAP2_TEXTURE_COORD_1,1,2,4,0,1,2,2,2,dv));	/* bad u order */
    TESTITV(glMap2d(GL_MAP2_TEXTURE_COORD_1,1,2,4,99,1,2,2,2,dv));	/* bad u order */
    TESTITV(glMap2d(GL_MAP2_TEXTURE_COORD_1,1,2,4,2,2,2,2,2,dv));	/* v1 = v2 */
    TESTITV(glMap2d(GL_MAP2_TEXTURE_COORD_1,1,2,4,2,1,2,0,2,dv));	/* bad v stride */
    TESTITV(glMap2d(GL_MAP2_TEXTURE_COORD_1,1,2,4,2,1,2,2,0,dv));	/* bad v order */
    TESTITV(glMap2d(GL_MAP2_TEXTURE_COORD_1,1,2,4,2,1,2,2,99,dv));	/* bad v order */

    TESTITE(glMap2f(-1,1,2,4,2,1,2,2,2,fv));				/* bad map */
    TESTITV(glMap2f(GL_MAP2_TEXTURE_COORD_1,2,2,4,2,1,2,2,2,fv));	/* u1 = u2 */
    TESTITV(glMap2f(GL_MAP2_TEXTURE_COORD_1,1,2,0,2,1,2,2,2,fv));	/* bad u stride */
    TESTITV(glMap2f(GL_MAP2_TEXTURE_COORD_1,1,2,4,0,1,2,2,2,fv));	/* bad u order */
    TESTITV(glMap2f(GL_MAP2_TEXTURE_COORD_1,1,2,4,99,1,2,2,2,fv));	/* bad u order */
    TESTITV(glMap2f(GL_MAP2_TEXTURE_COORD_1,1,2,4,2,2,2,2,2,fv));	/* v1 = v2 */
    TESTITV(glMap2f(GL_MAP2_TEXTURE_COORD_1,1,2,4,2,1,2,0,2,fv));	/* bad v stride */
    TESTITV(glMap2f(GL_MAP2_TEXTURE_COORD_1,1,2,4,2,1,2,2,0,fv));	/* bad v order */
    TESTITV(glMap2f(GL_MAP2_TEXTURE_COORD_1,1,2,4,2,1,2,2,99,fv));	/* bad v order */

    TESTITV(glMapGrid1d(-1,1.,10.));					/* bad un */
    TESTITV(glMapGrid1f(-1,1.,10.));					/* bad un */
    TESTITV(glMapGrid2d(-1,1.,10.,20,1.,10.));				/* bad un */
    TESTITV(glMapGrid2d(20,1.,10.,-1,1.,10.));				/* bad vn */
    TESTITV(glMapGrid2f(-1,1.,10.,20,1.,10.));				/* bad un */
    TESTITV(glMapGrid2f(20,1.,10.,-1,1.,10.));				/* bad vn */
    TESTITE(glEvalMesh1(-1,1,2));					/* bad mode */
    TESTITE(glEvalMesh2(-1,1,2,1,2));					/* bad mode */

    TESTITE(glAlphaFunc(-1,0.5));			/* bad func */
    TESTITE(glBlendFunc(-1,GL_ONE));			/* bad func */
    TESTITE(glBlendFunc(GL_ZERO,-1));			/* bad func */
    TESTITE(glLogicOp(-1));				/* bad func */
    TESTITE(glStencilFunc(-1,0.5,0x25));		/* bad func */
    TESTITE(glStencilOp(-1,GL_INVERT,GL_INVERT));	/* bad op */
    TESTITE(glStencilOp(GL_INVERT,-1,GL_INVERT));	/* bad op */
    TESTITE(glStencilOp(GL_INVERT,GL_INVERT,-1));	/* bad op */
    TESTITE(glDepthFunc(-1));				/* bad func */
    TESTITE(glPixelTransferf(-1,1.));			/* bad pname */
    TESTITE(glPixelTransferi(-1,1));			/* bad pname */

    TESTITE(glPixelMapfv(-1,4,fv));			/* bad map */
    TESTITV(glPixelMapfv(GL_PIXEL_MAP_I_TO_G,-1,fv));	/* bad size */
    TESTITV(glPixelMapfv(GL_PIXEL_MAP_I_TO_G,3,fv));	/* non ^2 size */
#if 0
    TESTITV(glPixelMapfv(GL_PIXEL_MAP_I_TO_G,1<<20,fv));	/* really big size */
#endif

    TESTITE(glPixelMapuiv(-1,4,uiv));			/* bad map */
    TESTITV(glPixelMapuiv(GL_PIXEL_MAP_I_TO_G,-1,uiv));	/* bad size */
    TESTITV(glPixelMapuiv(GL_PIXEL_MAP_I_TO_G,3,uiv));	/* non ^2 size */
#if 0
    TESTITV(glPixelMapuiv(GL_PIXEL_MAP_I_TO_G,1<<20,uiv));	/* really big size */
#endif

    TESTITE(glPixelMapusv(-1,4,usv));			/* bad map */
    TESTITV(glPixelMapusv(GL_PIXEL_MAP_I_TO_G,-1,usv));	/* bad size */
    TESTITV(glPixelMapusv(GL_PIXEL_MAP_I_TO_G,3,usv));	/* non ^2 size */
#if 0
    TESTITV(glPixelMapusv(GL_PIXEL_MAP_I_TO_G,1<<20,usv));	/* really big size */
#endif

    TESTITE(glReadBuffer(GL_ONE));			/* bad pname */
    if (aux_buffers == 0)
	TESTITO(glReadBuffer(GL_AUX0)); /* bad operation */
    else
	TESTITO(glReadBuffer(GL_AUX0 + aux_buffers)); /* bad operation */
    TESTITO(glReadBuffer(GL_AUX0+299));         /* bad operation */
    TESTITE(glCopyPixels(10,10,10,10,-1));		/* bad type */
    TESTITV(glCopyPixels(10,10,-1,10,GL_COLOR));	/* bad width */
    TESTITV(glCopyPixels(10,10,10,-1,GL_COLOR));	/* bad height */

#if 0
    /*XXXblythe fix me*/
    TESTIT(glDrawPixels(2,2,GL_BITMAP,GL_COLOR_INDEX,stipple));
#endif

    TESTITV(glFrustum(-2.,2.,-2.,2.,-0.1,1.));		/* negative near */
    TESTITV(glFrustum(-2.,2.,-2.,2.,0.1,-1.));		/* negative far */
    TESTITV(glFrustum(2.,2.,-2.,2.,0.1,1.));		/* delta x = 0 */
    TESTITV(glFrustum(-2.,2.,2.,2.,0.1,1.));		/* delta y = 0 */
    TESTITV(glFrustum(-2.,2.,-2.,2.,1.,1.));		/* delta z = 0 */
    TESTITE(glMatrixMode(-1));				/* bad mode */
    TESTITV(glOrtho(2.,2.,-2.,2.,-1.,1.));		/* delta x = 0 */
    TESTITV(glOrtho(-2.,2.,2.,2.,-1.,1.));		/* delta y = 0 */
    TESTITV(glOrtho(-2.,2.,-2.,2.,1.,1.));		/* delta z = 0 */
    TESTITV(glViewport(10,10,-1,40));			/* negative width */
    TESTITV(glViewport(10,10,40,-1));			/* negative height */
}

CLEANUP(errors) {
    glDeleteLists(1, 1);
    /* no other cleanup should be necessary, since errors shouldn't
     * cause state changes
     */
}

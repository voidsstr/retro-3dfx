/**************************************************************************
 *									  *
 * 		 Copyright (C) 1994, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
 *  pushattrib.c - $Revision: 2$
 *
 *
 *  Test glPushAttrib/PopAttrib.
 *
 *  how it works:
 *  two complete structures containing all (well, maybe most) of opengl state
 *  are used, one containing the default state, another an alternate state.
 *
 *  first, default state is verified, via "Get" calls.
 *  the state is "pushed".
 *  then, some state is changed.  these changes are verified.
 *  the state is "popped".
 *  last, the defaults state is again verified.
 */
#include <stdio.h>
#include <strings.h>
#include "ogtst.h"
#include "pushattrib.h"


#define SETRGBA(p, rr, gg, bb, aa) { p.r = rr; p.g = gg; p.b = bb; p.a = aa; }
#define SETXYZW(p, rr, gg, bb, aa) { p.x = rr; p.y = gg; p.z = bb; p.w = aa; }
#define SETSTRQ(p, rr, gg, bb, aa) { p.s = rr; p.t = gg; p.r = bb; p.q = aa; }

static OpenGLAttrib SDefault, SAlternate;

#define CHK_DEFAULT	0
#define CHK_ALTERNATE	1
static void checkstate(int flag, GLenum bit, char *extraMsg);
static void expecterror(GLenum code, char *msg);
static void doattribbits(void);
static void initdefaultstate(void);
static void initalternatestate(void);
static char *attrib2str(GLenum bits);

/* flags for extensions (availability checked at runtime) */
static int blend_minmax_extension;
static int convolution_extension;
static int ffd_extension;
static int histogram_extension;
static int multisample_extension;
static int polygon_offset_extension;
static int texture_add_env_extension;
static int texture_select_extension;
static int texture_scale_bias_extension;
static int reference_plane_extension;

static int rgbamode;
static int buffersize;

/*ARGSUSED*/
TESTMOD(pushattrib)
{
    int i, maxdepth, depth;
    const char *extensions = (const char *) glGetString(GL_EXTENSIONS);

    rgbamode = ogEnvCurVisualInfo(GLX_RGBA);
    buffersize = rgbamode ? 0 : ogEnvCurVisualInfo(GLX_BUFFER_SIZE);

#define USE_EXTENSION(NAME, PREFIX)                             \
    NAME##_extension = extensions != NULL &&                    \
        strstr(extensions, "GL_" #PREFIX "_" #NAME) != NULL;    \
    if (NAME##_extension)                                       \
	ogEnvLog(1, "using " #NAME " extension.\n")
       

    USE_EXTENSION(polygon_offset, EXT);

    initdefaultstate();
    initalternatestate();
    
    /*
     *  test each attrib bit
     */
    doattribbits();
    /*
     *  test some other aspects of attribute stack (e.g. Gets...)
     */
    glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH, &maxdepth);
    if (maxdepth < 16) {
        ogEnvLog(OG_LFAIL,
                 "MAX_ATTRIB_STACK_DEPTH of %d below the opengl minimum!\n",
                 maxdepth);
    }
    ogEnvLog(1, "Push to maxdepth with one attrib\n");
    for (i = 0; i < maxdepth; i++) {
        ogEnvLog(2, "glPushAttrib(GL_ACCUM_BUFFER_BIT);\n");
        glPushAttrib(GL_ACCUM_BUFFER_BIT);
        if (i == 8) {
            glGetIntegerv(GL_ATTRIB_STACK_DEPTH, &depth);
            if (depth != 9) {
                ogEnvLog(OG_LFAIL,
                         "ERROR: got depth %d (expected 9) after 8 \
glPushAttrib\n", depth);
            }
        }
    }
    expecterror(GL_NO_ERROR, "after 'max' pushes");
    ogEnvLog(1, "Pop all\n");
    for (i = 0; i < maxdepth; i++) {
        ogEnvLog(2, "glPopAttrib();\n");
        glPopAttrib();
    }
    expecterror(GL_NO_ERROR, "after 'max' pops");

    ogEnvLog(1, "Test for underflow\n");
    glPopAttrib();
    ogEnvLog(2, "glPopAttrib();\n");
    expecterror(GL_STACK_UNDERFLOW, "underflow");

    ogEnvLog(1, "push to max depth with all attribs\n");

    for (i = 0; i < maxdepth; i++) {
        {
            ogEnvLog(2, "PushAttrib((ALL_ATTRIB_BITS)\n");
            glPushAttrib(GL_ALL_ATTRIB_BITS);
        }

    }

    expecterror(GL_NO_ERROR, "after push of 'all' state");

    ogEnvLog(1, "Pop all\n");
#define GET_ERROR do {                                          \
        GLenum error;                                           \
        if ((error = glGetError()) != GL_NO_ERROR)              \
        fprintf(stderr, "Got error %s at line %d i %d\n",       \
                gluErrorString(error), __LINE__, i);            \
    } while (0)
    for (i = 0; i < maxdepth; i++) {
        ogEnvLog(2, "glPopAttrib();\n");
        glPopAttrib();
        GET_ERROR;
    }
    expecterror(GL_NO_ERROR, "after pop of 'all' state");
    glGetIntegerv(GL_ATTRIB_STACK_DEPTH, &depth);
    if (depth != 0) {
        ogEnvLog(OG_LFAIL,
                 "ERROR: got depth %d (expected 0) after all pops\n",
                 depth);
    }
	{
	    checkstate(CHK_DEFAULT, GL_ALL_ATTRIB_BITS, "After all pops");
	}
}

CLEANUP(pushattrib)
{
    int i;
    GLubyte p[32*4];

    for (i = 0; i < 32*4; i++)
        p[i] = 0xff;
    glPolygonStipple(p);
}

/*
 *  Enabling COLOR_MATERIAL change the AMBIENT and DIFFUSE materials.
 */
static void
restore_ambient_diffuse(void)
{
    float c[4];

    c[0] = .2f; c[1] = .2f; c[2] = .2f; c[3] = 1.f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);
    c[0] = .8f; c[1] = .8f; c[2] = .8f; c[3] = 1.f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);
}


#if 0
static void
loadRandomMatrices()
{
    GLfloat matrix[16];

    for (i = 0; i < 16; i++)
        matrix[i] = ogLibFloatRand(-10, 10);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(matrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(matrix);
    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(matrix);
    glMatrixMode(GL_MODELVIEW);
}
#endif

static void 
doattribbits(void)
{
#   define BIT_PROLOG(BIT)                              \
    checkstate(CHK_DEFAULT, GL_##BIT, "Before Push");   \
    ogEnvLog(2, "glPushAttrib(GL_" #BIT ");\n");        \
    glPushAttrib(GL_##BIT)
#   define BIT_EPILOG(BIT)                              \
    ogEnvLog(2, "glPopAttrib();\n");                    \
    glPopAttrib();                                      \
    checkstate(CHK_DEFAULT, GL_##BIT, "After Pop")
#   define ENABLE_DISABLE(ENUM, VAR) \
    if (SAlternate.enables.VAR) glEnable(GL_##ENUM); else glDisable(GL_##ENUM)

    /* accum buffer */
    {
    _GLaccumState *A = &SAlternate.accum;
    BIT_PROLOG(ACCUM_BUFFER_BIT);
    glClearAccum(A->aclear.r, A->aclear.g, A->aclear.b, A->aclear.a);
    BIT_EPILOG(ACCUM_BUFFER_BIT);
    }

    /* color buffer */
    {
    _GLrasterState *R = &SAlternate.raster;
    BIT_PROLOG(COLOR_BUFFER_BIT);
    ENABLE_DISABLE(ALPHA_TEST, alpha_test);
    ENABLE_DISABLE(BLEND, blend);
    ENABLE_DISABLE(DITHER, dither);
    ENABLE_DISABLE(LOGIC_OP, logic_op);
    if (rgbamode)
	glColorMask(R->rgbaMask[0], R->rgbaMask[1],
                    R->rgbaMask[2], R->rgbaMask[3]);
    else
	glIndexMask(R->writeMask);
    glClearColor(R->cclear.r, R->cclear.g, R->cclear.b, R->cclear.a);
    glDrawBuffer(R->drawBuffer);
    BIT_EPILOG(COLOR_BUFFER_BIT);
    }

    /* current */
    {
    _GLcurrentState *C = &SAlternate.current;

    BIT_PROLOG(CURRENT_BIT);
    /* for rgba visuals color index won't change and vice versa */
    if (rgbamode) {
	glColor4fv(&C->color.r);
    } else {
	glIndexf(C->index);
    }
    glNormal3fv(&C->normal.x);
    glTexCoord4fv(&C->texCoord.s);
    glRasterPos4fv(&C->rasterPos.x);
    BIT_EPILOG(CURRENT_BIT);
    }

    /* depth buffer */
    {
    _GLdepthState *D = &SAlternate.depth;
    BIT_PROLOG(DEPTH_BUFFER_BIT);
    glDepthFunc(D->depthFunc);
    glDepthMask(D->writeEnable);
    glClearDepth(D->zclear);
    BIT_EPILOG(DEPTH_BUFFER_BIT);
    }

    /* enable */
    {
    int i;

    BIT_PROLOG(ENABLE_BIT);
    ENABLE_DISABLE(ALPHA_TEST, alpha_test);
    ENABLE_DISABLE(AUTO_NORMAL, auto_normal);
    ENABLE_DISABLE(BLEND, blend);
    ENABLE_DISABLE(COLOR_MATERIAL, color_material);
    ENABLE_DISABLE(CULL_FACE, cull_face);
    ENABLE_DISABLE(DEPTH_TEST, depth_test);
    ENABLE_DISABLE(FOG, fog);
    ENABLE_DISABLE(LINE_SMOOTH, line_smooth);
    ENABLE_DISABLE(LINE_STIPPLE, line_stipple);
    ENABLE_DISABLE(LOGIC_OP, logic_op);
    ENABLE_DISABLE(NORMALIZE, normalize);
    ENABLE_DISABLE(POINT_SMOOTH, point_smooth);
    ENABLE_DISABLE(POLYGON_SMOOTH, polygon_smooth);
    ENABLE_DISABLE(POLYGON_STIPPLE, polygon_stipple);
    ENABLE_DISABLE(SCISSOR_TEST, scissor_test);
    ENABLE_DISABLE(STENCIL_TEST, stencil_test);
    ENABLE_DISABLE(TEXTURE_2D, texture2d);
    ENABLE_DISABLE(TEXTURE_GEN_S, tex_gen_s);
    ENABLE_DISABLE(TEXTURE_GEN_T, tex_gen_t);
    ENABLE_DISABLE(TEXTURE_GEN_R, tex_gen_r);
    ENABLE_DISABLE(TEXTURE_GEN_Q, tex_gen_q);

    ENABLE_DISABLE(MAP1_COLOR_4, eval1d.color4);
    ENABLE_DISABLE(MAP1_INDEX, eval1d.index);
    ENABLE_DISABLE(MAP1_NORMAL, eval1d.normal);
    ENABLE_DISABLE(MAP1_TEXTURE_COORD_1, eval1d.textureCoord1);
    ENABLE_DISABLE(MAP1_TEXTURE_COORD_2, eval1d.textureCoord2);
    ENABLE_DISABLE(MAP1_TEXTURE_COORD_4, eval1d.textureCoord4);
    ENABLE_DISABLE(MAP1_VERTEX_3, eval1d.vertex3);    
    ENABLE_DISABLE(MAP1_VERTEX_4, eval1d.vertex4);    

    ENABLE_DISABLE(MAP2_COLOR_4, eval2d.color4);
    ENABLE_DISABLE(MAP2_INDEX, eval2d.index);
    ENABLE_DISABLE(MAP2_NORMAL, eval2d.normal);
    ENABLE_DISABLE(MAP2_TEXTURE_COORD_1, eval2d.textureCoord1);
    ENABLE_DISABLE(MAP2_TEXTURE_COORD_2, eval2d.textureCoord2);
    ENABLE_DISABLE(MAP2_TEXTURE_COORD_4, eval2d.textureCoord4);
    ENABLE_DISABLE(MAP2_VERTEX_3, eval2d.vertex3);    
    ENABLE_DISABLE(MAP2_VERTEX_4, eval2d.vertex4);    

    for (i = 0; i < _GL_NUM_LIGHTS; i++)
        if (SAlternate.enables.lights[i])
            glEnable(GL_LIGHT0+i);
        else
            glDisable(GL_LIGHT0+i);
    ENABLE_DISABLE(LIGHTING, lighting);

    for (i = 0; i < _GL_NUM_CLIP_PLANES; i++)
        if (SAlternate.enables.lights[i])
            glEnable(GL_CLIP_PLANE0+i);
        else
            glDisable(GL_CLIP_PLANE0+i);

#if defined(GL_EXT_polygon_offset)
    if (polygon_offset_extension)
	ENABLE_DISABLE(POLYGON_OFFSET_EXT, polygon_offset);
#endif
    BIT_EPILOG(ENABLE_BIT);
    restore_ambient_diffuse();
    }

    /* eval */
    {
    BIT_PROLOG(EVAL_BIT);

    ENABLE_DISABLE(AUTO_NORMAL, auto_normal);

    ENABLE_DISABLE(MAP1_COLOR_4, eval1d.color4);
    ENABLE_DISABLE(MAP1_INDEX, eval1d.index);
    ENABLE_DISABLE(MAP1_NORMAL, eval1d.normal);
    ENABLE_DISABLE(MAP1_TEXTURE_COORD_1, eval1d.textureCoord1);
    ENABLE_DISABLE(MAP1_TEXTURE_COORD_2, eval1d.textureCoord2);
    ENABLE_DISABLE(MAP1_TEXTURE_COORD_4, eval1d.textureCoord4);
    ENABLE_DISABLE(MAP1_VERTEX_3, eval1d.vertex3);    
    ENABLE_DISABLE(MAP1_VERTEX_4, eval1d.vertex4);    

    ENABLE_DISABLE(MAP2_COLOR_4, eval2d.color4);
    ENABLE_DISABLE(MAP2_INDEX, eval2d.index);
    ENABLE_DISABLE(MAP2_NORMAL, eval2d.normal);
    ENABLE_DISABLE(MAP2_TEXTURE_COORD_1, eval2d.textureCoord1);
    ENABLE_DISABLE(MAP2_TEXTURE_COORD_2, eval2d.textureCoord2);
    ENABLE_DISABLE(MAP2_TEXTURE_COORD_4, eval2d.textureCoord4);
    ENABLE_DISABLE(MAP2_VERTEX_3, eval2d.vertex3);    
    ENABLE_DISABLE(MAP2_VERTEX_4, eval2d.vertex4);    

    glMapGrid1f(SAlternate.evals.segments1d,
                SAlternate.evals.domain1d[0], SAlternate.evals.domain1d[1]);
    glMapGrid2f(SAlternate.evals.segments2d[0],
                SAlternate.evals.domain2d[0][0],
                SAlternate.evals.domain2d[0][1],
                SAlternate.evals.segments2d[1],
                SAlternate.evals.domain2d[1][0],
                SAlternate.evals.domain2d[1][1]);
    BIT_EPILOG(EVAL_BIT);
    }

    /* fog */
    {
    _GLfogState *F = &SAlternate.fog;
    BIT_PROLOG(FOG_BIT);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    if (rgbamode) {
	glFogfv(GL_FOG_COLOR, &F->color.r);
    } else {
	glFogf(GL_FOG_INDEX, F->index);
    }
    glFogf(GL_FOG_DENSITY, F->density);
    glFogf(GL_FOG_START, F->start);
    glFogf(GL_FOG_END, F->end);
    BIT_EPILOG(FOG_BIT);
    }

    /* hint */
    {
    _GLhintState *H = &SAlternate.hints;
    BIT_PROLOG(HINT_BIT);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, H->perspectiveCorrection);
    glHint(GL_POINT_SMOOTH_HINT, H->pointSmooth);
    glHint(GL_LINE_SMOOTH_HINT, H->lineSmooth);
    glHint(GL_POLYGON_SMOOTH_HINT, H->polygonSmooth);
    glHint(GL_FOG_HINT, H->fog);
    BIT_EPILOG(HINT_BIT);
    }

    /* lighting */
    {
    _GLmaterialState *M;
    GLenum side;
    int i;

    BIT_PROLOG(LIGHTING_BIT);

    M = &SAlternate.light.front;
    side = GL_FRONT;
    for (i = 0; i < 2; i++) {
	glMaterialfv(side, GL_AMBIENT, (float*)&M->ambient.r);
	glMaterialfv(side, GL_DIFFUSE, (float*)&M->diffuse.r);
	glMaterialfv(side, GL_SPECULAR, (float*)&M->specular.r);
	glMaterialfv(side, GL_EMISSION, (float*)&M->emission.r);
	glMaterialf(side, GL_SHININESS, M->shininess);
	glMaterialfv(side, GL_COLOR_INDEXES, M->cmap);

	M = &SAlternate.light.back;
	side = GL_BACK;
    }
    /*
     * Can not test enable COLOR_MATERIAL as it would change
     * the materials !!
     */
#ifdef XXXNYI
    glShadeModel(GL_FLAT);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lm_amb);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
    glLightfv(GL_LIGHT0, GL_POSITION, lt0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lt0_amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lt0_diff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lt0_spec);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, .11);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, .22);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, .33);
    glLightfv(GL_LIGHT7, GL_POSITION, lt7_pos);
    glLightfv(GL_LIGHT7, GL_SPOT_DIRECTION, lt7_spotdir);
    glLightf(GL_LIGHT7, GL_SPOT_EXPONENT, 100.0);
    glLightf(GL_LIGHT7, GL_SPOT_CUTOFF, 45.0);
    glLightfv(GL_LIGHT7, GL_AMBIENT, lt7_amb);
    glLightf(GL_LIGHT7, GL_CONSTANT_ATTENUATION, .44);
    glLightf(GL_LIGHT7, GL_LINEAR_ATTENUATION, .55);
    glLightf(GL_LIGHT7, GL_QUADRATIC_ATTENUATION, .66);
#endif /*XXXNYI*/
    BIT_EPILOG(LIGHTING_BIT);
    }

    /* line */
    {
    _GLlineState *L = &SAlternate.line;
    BIT_PROLOG(LINE_BIT);
    ENABLE_DISABLE(LINE_SMOOTH, line_smooth);
    ENABLE_DISABLE(LINE_STIPPLE, line_stipple);
    glLineStipple(L->stippleRepeat, L->stipple);
    glLineWidth(L->smoothWidth);
    BIT_EPILOG(LINE_BIT);
    }

    /* list */
    {
    _GLlistState *L = &SAlternate.list;
    BIT_PROLOG(LIST_BIT);
    glListBase(L->listBase);
    BIT_EPILOG(LIST_BIT);
    }

    /* pixel mode */
    {
    _GLpixelState *P = &SAlternate.pixel;

    BIT_PROLOG(PIXEL_MODE_BIT);
    glPixelTransferi(GL_MAP_COLOR, P->mapColor);
    glPixelTransferi(GL_MAP_STENCIL, P->mapStencil);
    glPixelTransferi(GL_INDEX_SHIFT, P->indexShift);
    glPixelTransferi(GL_INDEX_OFFSET, P->indexOffset);
    glPixelTransferf(GL_RED_SCALE, P->redScale);
    glPixelTransferf(GL_GREEN_SCALE, P->greenScale);
    glPixelTransferf(GL_BLUE_SCALE, P->blueScale);
    glPixelTransferf(GL_ALPHA_SCALE, P->alphaScale);
    glPixelTransferf(GL_DEPTH_SCALE, P->depthScale);
    glPixelTransferf(GL_RED_BIAS, P->redBias);
    glPixelTransferf(GL_GREEN_BIAS, P->greenBias);
    glPixelTransferf(GL_BLUE_BIAS, P->blueBias);
    glPixelTransferf(GL_ALPHA_BIAS, P->alphaBias);
    glPixelTransferf(GL_DEPTH_BIAS, P->depthBias);
    glPixelZoom(P->xZoom, P->yZoom);
    glReadBuffer(P->readBuffer);
    BIT_EPILOG(PIXEL_MODE_BIT);
    }

    /* point */
    {
    _GLpointState *P = &SAlternate.point;
    BIT_PROLOG(POINT_BIT);
    ENABLE_DISABLE(POINT_SMOOTH, point_smooth);
    glPointSize(P->smoothSize);
    BIT_EPILOG(POINT_BIT);
    }

    /* polygon */
    {
    _GLpolygonState *P = &SAlternate.polygon;
    BIT_PROLOG(POLYGON_BIT);
    ENABLE_DISABLE(POLYGON_SMOOTH, polygon_smooth);
    ENABLE_DISABLE(POLYGON_STIPPLE, polygon_stipple);
    ENABLE_DISABLE(CULL_FACE, cull_face);
    glCullFace(P->cullMode);
    glFrontFace(P->frontFaceDirection);
    glPolygonMode(GL_FRONT, P->frontMode);
    glPolygonMode(GL_BACK, P->backMode);
    BIT_EPILOG(POLYGON_BIT);
    }

    /* polygon stipple */
    {
    BIT_PROLOG(POLYGON_STIPPLE_BIT);
    glPolygonStipple(SAlternate.polystipple);
    BIT_EPILOG(POLYGON_STIPPLE_BIT);
    }

    /* scissor */
    {
    _GLscissorState *S = &SAlternate.scissor;
    BIT_PROLOG(SCISSOR_BIT);
    ENABLE_DISABLE(SCISSOR_TEST, scissor_test);
    glScissor(S->x, S->y, S->width, S->height);
    BIT_EPILOG(SCISSOR_BIT);
    }

    /* stencil buffer */
    {
/*XXX more needed! */
    _GLstencilState *S = &SAlternate.stencil;
    BIT_PROLOG(STENCIL_BUFFER_BIT);
    glStencilMask(S->stencilMask);
    BIT_EPILOG(STENCIL_BUFFER_BIT);
    }

    /* texture */
    {
    float array[4];
    BIT_PROLOG(TEXTURE_BIT);
    ENABLE_DISABLE(TEXTURE_2D, texture2d);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    array[0] = .5; array[1] = 0; array[2] = 1; array[3] = 0.1;
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, array);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    array[0] = .4; array[1] = .6; array[2] = .4; array[3] = .1;
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, array);

    glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGenf(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGenf(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

    BIT_EPILOG(TEXTURE_BIT);
    }

    /* XXX transform */
    {
    _GLtransformState *T = &SAlternate.transform;
    int i;

    BIT_PROLOG(TRANSFORM_BIT);
    ENABLE_DISABLE(NORMALIZE, normalize);
    for (i = 0; i < _GL_NUM_CLIP_PLANES; i++)
        if (SAlternate.enables.lights[i])
            glEnable(GL_CLIP_PLANE0+i);
        else
            glDisable(GL_CLIP_PLANE0+i);
    for (i = 0; i < _GL_NUM_CLIP_PLANES; i++) {
	glClipPlane(GL_CLIP_PLANE0+i, T->clipPlanes[i]);
    }
    glMatrixMode(T->matrixMode);
    BIT_EPILOG(TRANSFORM_BIT);
    }

    /* viewport */
    {
    _GLviewportState *V = &SAlternate.viewport;
    BIT_PROLOG(VIEWPORT_BIT);
    glViewport(V->x, V->y, V->width, V->height);
    glDepthRange(V->zNear, V->zFar);
    BIT_EPILOG(VIEWPORT_BIT);
    }

#ifdef XXXNYI
    /* all */
    {
    BIT_PROLOG(ALL_ATTRIB_BITS);
    checkstate(CHK_DEFAULT, GL_ALL_ATTRIB_BITS);
    BIT_EPILOG(ALL_ATTRIB_BITS);
    }
#endif /*XXXNYI*/
}


/* error threshold ~= 1/256 ==> good for 8 bit visuals? */
#define EPS .004

#define FLOATEQ(a, b)	(fabs((float)(a - b)) < EPS)
#define DOUBLEEQ(a, b)  FLOATEQ(a, b)
#define FLOATEQ4(A, B)                                          \
    (FLOATEQ((A)[0], (B)[0]) && FLOATEQ((A)[1], (B)[1]) &&      \
     FLOATEQ((A)[2], (B)[2]) && FLOATEQ((A)[3], (B)[3]))
#define DOUBLEEQ2(A, B) (DOUBLEEQ((A)[0], (B)[0]) && DOUBLEEQ((A)[1], (B)[1]))
#define DOUBLEEQ4(A, B)                                         \
    (DOUBLEEQ((A)[0], (B)[0]) && DOUBLEEQ((A)[1], (B)[1]) &&    \
     DOUBLEEQ((A)[2], (B)[2]) && DOUBLEEQ((A)[3], (B)[3]))
#define EQ4(A, B) \
     ((A)[0]==(B)[0] && (A)[1]==(B)[1] && (A)[2]==(B)[2] && (A)[3]==(B)[3])

static int
floatEqN(float A[], float B[], int n)
{
    int i;
    for (i = 0; i < n; i++) {
	if (!FLOATEQ(A[i], B[i]))
	    return 0;
    }
    return 1;
}

static char *stateNames[] = {
    "default",
    "alternate"
};

static void
checkstate(int state, GLenum bits, char *extraMsg)
{
    OpenGLAttrib *GSP;
    GLfloat v[4];
    GLdouble vd[4];
    GLboolean vb[4];
    int i, vi[4];
    char msg[128];
    char str[40];
#   define CHKERROR(ok, msg)                                    \
    if (!(ok))                                                  \
        ogEnvLog(OG_LFAIL, "%s(%s): %s\n",                      \
                 attrib2str(bits), stateNames[state],  msg)
#   define CHK_ENABLE(ENUM, VAR) \
    CHKERROR(glIsEnabled(GL_##ENUM) == GSP->enables.VAR, #ENUM);

    sprintf(msg, "checkstate: attrib bit %s (%s:%s)", attrib2str(bits),
            stateNames[state], extraMsg);
    ogEnvLog(1, "%s\n", msg);

    expecterror(GL_NO_ERROR, msg);
    /* do comparisons with one of two state structures */
    GSP = (state == CHK_DEFAULT) ? &SDefault : &SAlternate;

    if (bits & GL_ACCUM_BUFFER_BIT) {
	glGetFloatv(GL_ACCUM_CLEAR_VALUE, v);
        CHKERROR(FLOATEQ4(v, (float*)&GSP->accum.aclear.r), "accum clear value");
    }
    if (bits & GL_COLOR_BUFFER_BIT) {
	_GLrasterState *R = &GSP->raster;
        CHK_ENABLE(ALPHA_TEST, alpha_test);
        CHK_ENABLE(BLEND, blend);
        CHK_ENABLE(DITHER, dither);
        CHK_ENABLE(LOGIC_OP, logic_op);
	glGetIntegerv(GL_ALPHA_TEST_FUNC, vi);
	CHKERROR(vi[0] == R->alphaFunc, "alphaFunc");
	glGetFloatv(GL_ALPHA_TEST_REF, v);
	CHKERROR(FLOATEQ(v[0], R->alphaRef), "alphaRef");
	glGetIntegerv(GL_BLEND_SRC, vi);
	CHKERROR(vi[0] == R->blendSrc, "blendSrc");
	glGetIntegerv(GL_BLEND_DST, vi);
	CHKERROR(vi[0] == R->blendDst, "blendDst");
	glGetIntegerv(GL_LOGIC_OP_MODE, vi);
	CHKERROR(vi[0] == R->logicOp, "logicOpOp");
	glGetIntegerv(GL_DRAW_BUFFER, vi);
	CHKERROR(vi[0] == R->drawBuffer, "drawBuffer");
	if (rgbamode) {
            glGetBooleanv(GL_COLOR_WRITEMASK, vb);
	    CHKERROR(EQ4(vb, &R->rgbaMask[0]), "rgbaMask");
	} else {
            glGetIntegerv(GL_INDEX_WRITEMASK, vi);
	    CHKERROR(vi[0] == R->writeMask, "IndexWriteMask");
	}
	glGetFloatv(GL_COLOR_CLEAR_VALUE, v);
	CHKERROR(FLOATEQ4(v, (float*)&R->cclear.r), "cclear");
	glGetIntegerv(GL_INDEX_CLEAR_VALUE, vi);
	CHKERROR(vi[0] == R->clearIndex, "clearIndex");
    }
    if (bits & GL_CURRENT_BIT) { 
	_GLcurrentState *C = &GSP->current;

	if (rgbamode) {
            glGetFloatv(GL_CURRENT_COLOR, v);
            CHKERROR(FLOATEQ4(v, &C->color.r), "color");
	} else {
            glGetFloatv(GL_CURRENT_INDEX, v);
            CHKERROR(FLOATEQ(v[0], C->index), "index");
	}
	glGetFloatv(GL_CURRENT_NORMAL, v);
	CHKERROR(floatEqN(v, &C->normal.x, 3), "normal");
	glGetFloatv(GL_CURRENT_TEXTURE_COORDS, v);
	CHKERROR(FLOATEQ4(v, &C->texCoord.s), "texCoord");
	glGetIntegerv(GL_CURRENT_RASTER_POSITION_VALID, vi);
	CHKERROR(vi[0] == C->validRasterPos, "validRasterPos");
	glGetFloatv(GL_CURRENT_RASTER_POSITION, v);
	CHKERROR(FLOATEQ4(v, &C->rasterPos.x), "rasterPos");
	glGetFloatv(GL_CURRENT_RASTER_DISTANCE, v);
	CHKERROR(FLOATEQ(v[0], C->rasterZ), "rasterZ");
	if (rgbamode) {
            glGetFloatv(GL_CURRENT_RASTER_COLOR, v);
            CHKERROR(FLOATEQ4(v, &C->rasterColor.r), "rasterColor");
            glGetFloatv(GL_CURRENT_RASTER_TEXTURE_COORDS, v);
            CHKERROR(FLOATEQ4(v, &C->rasterTexCoord.s), "rasterTexCoord");
	} else {
            glGetFloatv(GL_CURRENT_RASTER_INDEX, v);
            CHKERROR(FLOATEQ(v[0], C->rasterIndex), "rasterIndex");
	}
    }
    if (bits & GL_DEPTH_BUFFER_BIT) {
	_GLdepthState *D = &GSP->depth;

	glGetFloatv(GL_DEPTH_CLEAR_VALUE, v);
	CHKERROR(FLOATEQ(v[0], D->zclear), "depth clear");
	glGetIntegerv(GL_DEPTH_FUNC, vi);
	CHKERROR(vi[0] == D->depthFunc, "depth func");
	glGetIntegerv(GL_DEPTH_WRITEMASK, vi);
        CHKERROR(vi[0] == D->writeEnable, "depth writemask");
    }
    if (bits & GL_ENABLE_BIT) {
	CHK_ENABLE(ALPHA_TEST, alpha_test);
	CHK_ENABLE(AUTO_NORMAL, auto_normal);
	CHK_ENABLE(BLEND, blend);
	CHK_ENABLE(COLOR_MATERIAL, color_material);
	CHK_ENABLE(CULL_FACE, cull_face);
	CHK_ENABLE(DEPTH_TEST, depth_test);
	CHK_ENABLE(DITHER, dither);
	CHK_ENABLE(FOG, fog);
	CHK_ENABLE(LIGHTING, lighting);
	for (i = 0; i < _GL_NUM_LIGHTS; i++) {
	    sprintf(str, "lights[%d]", i);
	    CHKERROR(glIsEnabled(GL_LIGHT0 + i) == GSP->enables.lights[i], str);
        }
	CHK_ENABLE(LINE_SMOOTH, line_smooth);
	CHK_ENABLE(LINE_STIPPLE, line_stipple);
	CHK_ENABLE(LOGIC_OP, logic_op);
	CHK_ENABLE(MAP1_COLOR_4, eval1d.color4);
	CHK_ENABLE(MAP1_INDEX, eval1d.index);
	CHK_ENABLE(MAP1_NORMAL, eval1d.normal);
	CHK_ENABLE(MAP1_TEXTURE_COORD_1, eval1d.textureCoord1);
	CHK_ENABLE(MAP1_TEXTURE_COORD_2, eval1d.textureCoord2);
	CHK_ENABLE(MAP1_TEXTURE_COORD_3, eval1d.textureCoord3);
	CHK_ENABLE(MAP1_TEXTURE_COORD_4, eval1d.textureCoord4);
	CHK_ENABLE(MAP1_VERTEX_3, eval1d.vertex3);
	CHK_ENABLE(MAP1_VERTEX_4, eval1d.vertex4);
	/* map2 */
	CHK_ENABLE(MAP2_COLOR_4, eval2d.color4);
	CHK_ENABLE(MAP2_INDEX, eval2d.index);
	CHK_ENABLE(MAP2_NORMAL, eval2d.normal);
	CHK_ENABLE(MAP2_TEXTURE_COORD_1, eval2d.textureCoord1);
	CHK_ENABLE(MAP2_TEXTURE_COORD_2, eval2d.textureCoord2);
	CHK_ENABLE(MAP2_TEXTURE_COORD_3, eval2d.textureCoord3);
	CHK_ENABLE(MAP2_TEXTURE_COORD_4, eval2d.textureCoord4);
	CHK_ENABLE(MAP2_VERTEX_3, eval2d.vertex3);
	CHK_ENABLE(MAP2_VERTEX_4, eval2d.vertex4);
	CHK_ENABLE(NORMALIZE, normalize);
	CHK_ENABLE(POINT_SMOOTH, point_smooth);
	CHK_ENABLE(POLYGON_SMOOTH, polygon_smooth);
	CHK_ENABLE(POLYGON_STIPPLE, polygon_stipple);
	CHK_ENABLE(SCISSOR_TEST, scissor_test);
	CHK_ENABLE(STENCIL_TEST, stencil_test);
	CHK_ENABLE(TEXTURE_1D, texture1d);
	CHK_ENABLE(TEXTURE_2D, texture2d);
	CHK_ENABLE(TEXTURE_GEN_S, tex_gen_s);
	CHK_ENABLE(TEXTURE_GEN_T, tex_gen_t);
	CHK_ENABLE(TEXTURE_GEN_R, tex_gen_r);
	CHK_ENABLE(TEXTURE_GEN_Q, tex_gen_q);
	for (i = 0; i < _GL_NUM_CLIP_PLANES; i++) {
	    sprintf(str, "clipPlanes %d", i);
	    CHKERROR(glIsEnabled(GL_CLIP_PLANE0 + i) ==
                     GSP->enables.clipPlanes[i], str);
	}
#if defined(GL_EXT_polygon_offset)
	if (polygon_offset_extension) {
            CHK_ENABLE(POLYGON_OFFSET_EXT, polygon_offset);
        }
#endif
    }
    if (bits & GL_EVAL_BIT) {
        /* map1 */
	CHK_ENABLE(MAP1_COLOR_4, eval1d.color4);
	CHK_ENABLE(MAP1_INDEX, eval1d.index);
	CHK_ENABLE(MAP1_NORMAL, eval1d.normal);
	CHK_ENABLE(MAP1_TEXTURE_COORD_1, eval1d.textureCoord1);
	CHK_ENABLE(MAP1_TEXTURE_COORD_2, eval1d.textureCoord2);
	CHK_ENABLE(MAP1_TEXTURE_COORD_3, eval1d.textureCoord3);
	CHK_ENABLE(MAP1_TEXTURE_COORD_4, eval1d.textureCoord4);
	CHK_ENABLE(MAP1_VERTEX_3, eval1d.vertex3);
	CHK_ENABLE(MAP1_VERTEX_4, eval1d.vertex4);
	glGetFloatv(GL_MAP1_GRID_DOMAIN, v);
	glGetIntegerv(GL_MAP1_GRID_SEGMENTS, vi);
        CHKERROR(v[0] == GSP->evals.domain1d[0], "MAP1_GRID_DOMAIN[0]");
        CHKERROR(v[1] == GSP->evals.domain1d[1], "MAP1_GRID_DOMAIN[1]");
        CHKERROR(vi[0] == GSP->evals.segments1d, "MAP1_GRID_SEGEMENTS");
	/* map2 */
	CHK_ENABLE(MAP2_COLOR_4, eval2d.color4);
	CHK_ENABLE(MAP2_INDEX, eval2d.index);
	CHK_ENABLE(MAP2_NORMAL, eval2d.normal);
	CHK_ENABLE(MAP2_TEXTURE_COORD_1, eval2d.textureCoord1);
	CHK_ENABLE(MAP2_TEXTURE_COORD_2, eval2d.textureCoord2);
	CHK_ENABLE(MAP2_TEXTURE_COORD_3, eval2d.textureCoord3);
	CHK_ENABLE(MAP2_TEXTURE_COORD_4, eval2d.textureCoord4);
	CHK_ENABLE(MAP2_VERTEX_3, eval2d.vertex3);
	CHK_ENABLE(MAP2_VERTEX_4, eval2d.vertex4);
	glGetFloatv(GL_MAP2_GRID_DOMAIN, v);
	glGetIntegerv(GL_MAP2_GRID_SEGMENTS, vi);
        for (i = 0; i < 2; i++) {
            sprintf(str, "MAP2_GRID_DOMAIN[%d][0]", i);
            CHKERROR(v[2 * i + 0] == GSP->evals.domain2d[i][0], str);
            sprintf(str, "MAP2_GRID_DOMAIN[%d][1]", i);
            CHKERROR(v[2 * i + 1] == GSP->evals.domain2d[i][1], str);
            sprintf(str, "MAP2_GRID_SEGEMENTS[%d]", i);
            CHKERROR(vi[i] == GSP->evals.segments2d[i], str);
        }
    }
    if (bits & GL_FOG_BIT) {
	_GLfogState *F = &GSP->fog;

	glGetFloatv(GL_FOG_DENSITY, v);
	CHKERROR(FLOATEQ(v[0], F->density), "density");
	glGetFloatv(GL_FOG_START, v);
	CHKERROR(FLOATEQ(v[0], F->start), "start");
	glGetFloatv(GL_FOG_END, v);
	CHKERROR(FLOATEQ(v[0], F->end), "end");
	glGetIntegerv(GL_FOG_MODE, vi);
	CHKERROR(vi[0] == F->mode, "mode");
	if (rgbamode) {
            glGetFloatv(GL_FOG_COLOR, v);
	    CHKERROR(FLOATEQ4(v, &F->color.r), "color");
	} else {
            glGetFloatv(GL_FOG_INDEX, v);
	    CHKERROR(FLOATEQ(v[0], F->index), "index");
	}
    }
    if (bits & GL_HINT_BIT) {
	_GLhintState *H = &GSP->hints;

	glGetIntegerv(GL_PERSPECTIVE_CORRECTION_HINT, vi);
	CHKERROR(vi[0] == H->perspectiveCorrection, "perspectiveCorrection");
	glGetIntegerv(GL_POINT_SMOOTH_HINT, vi);
	CHKERROR(vi[0] == H->pointSmooth, "pointSmooth");
	glGetIntegerv(GL_LINE_SMOOTH_HINT, vi);
	CHKERROR(vi[0] == H->lineSmooth, "lineSmooth");
	glGetIntegerv(GL_POLYGON_SMOOTH_HINT, vi);
	CHKERROR(vi[0] == H->polygonSmooth, "polygonSmooth");
	glGetIntegerv(GL_FOG_HINT, vi);
	CHKERROR(vi[0] == H->fog, "fog");
    }
    if (bits & GL_LIGHTING_BIT) {
	/* XXX still much checking is missing! (colorindex, lights, etc.) */
	_GLmaterialState *M = &GSP->light.front;
        static char *sides[] = {"FRONT", "BACK"};
	
	for (i = GL_FRONT; i <= GL_BACK; i++) {
#           define CHK_MAT4(S, A) do {                                  \
            glGetMaterialfv(i, GL_##A, v);                              \
            if (!FLOATEQ4(v, (float *) &S))                             \
                 ogEnvLog(OG_LFAIL, "LIGHTING(%s): " #A                 \
                          "(%s) (%g %g %g %g) != (%g %g %g %g)\n",      \
                          stateNames[state], sides[i - GL_FRONT],       \
                          v[0], v[1], v[2], v[3], ((float *) &S)[0],    \
                          ((float *) &S)[1], ((float *) &S)[2],         \
                          ((float *) &S)[3]);                           \
            } while (0)

            CHK_MAT4(M->ambient.r, AMBIENT);
            CHK_MAT4(M->diffuse.r, DIFFUSE);
            CHK_MAT4(M->specular.r, SPECULAR);
            CHK_MAT4(M->emission.r, EMISSION);
	    glGetMaterialfv(i, GL_SHININESS, v);
            if (v[0] != M->shininess)
                ogEnvLog(OG_LFAIL, "LIGHTING(%s): shininess(%s) %g != %g\n",
                         stateNames[state], sides[i - GL_FRONT],
                         v[0], M->shininess);
	    glGetMaterialfv(i, GL_COLOR_INDEXES, v);
            if (!floatEqN(v, M->cmap, 3))
                ogEnvLog(OG_LFAIL,
                         "LIGHTING_BIT(%s): color indices(%s) (%g %g %g) \
!= (%g %g %g)\n", stateNames[state], sides[i - GL_FRONT],
                         v[0], v[1], v[2], M->cmap[0], M->cmap[1], M->cmap[2]);
	    M = &GSP->light.back;
#           undef CHK_MAT4
        }
#ifdef XXXNYI
	glGetIntegerv(GL_SHADE_MODEL, vi);
	glGetIntegerv(GL_LIGHT_MODEL_TWO_SIDE, vi);
	glGetFloatv(GL_LIGHT_MODEL_AMBIENT, v4);

	for (i = 0; i < 7; i++) {
	    glGetLightfv(GL_LIGHT0 + i, GL_AMBIENT, v);
	    glGetLightfv(GL_LIGHT0 + i, GL_CONSTANT_ATTENUATION, v);
	    glGetLightfv(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, v);
	    glGetLightfv(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, v);
	    glGetLightfv(GL_LIGHT0 + i, GL_DIFFUSE, v);
	    glGetLightfv(GL_LIGHT0 + i, GL_SPECULAR, v);
	    glGetLightfv(GL_LIGHT0 + i, GL_POSITION, v);
	    glGetLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, v);
	    glGetLightfv(GL_LIGHT0 + i, GL_SPOT_EXPONENT, v);
	    glGetLightfv(GL_LIGHT0 + i, GL_SPOT_CUTOFF, v);
	}
#endif /*XXXNYI*/
    }
    if (bits & GL_LINE_BIT) {
	_GLlineState *L = &GSP->line;
	glGetFloatv(GL_LINE_WIDTH, v);
	CHKERROR(FLOATEQ(v[0], L->smoothWidth), "smoothWidth");
	glGetIntegerv(GL_LINE_STIPPLE_REPEAT, vi);
	CHKERROR(vi[0] == L->stippleRepeat, "stippleRepeat");
	glGetIntegerv(GL_LINE_STIPPLE_PATTERN, vi);
	CHKERROR(vi[0] == L->stipple, "stipple");
    }
    if (bits & GL_LIST_BIT) {
	_GLlistState *L = &GSP->list;
	glGetIntegerv(GL_LIST_BASE, vi);
	CHKERROR(vi[0] == L->listBase, "listBase");
    }
    if (bits & GL_PIXEL_MODE_BIT) {
	_GLpixelState *P = &GSP->pixel;

	glGetBooleanv(GL_MAP_COLOR, vb);
	CHKERROR(vb[0] == P->mapColor, "mapColor");
	glGetBooleanv(GL_MAP_STENCIL, vb);
	CHKERROR(vb[0] == P->mapStencil, "mapStencil");
	glGetIntegerv(GL_INDEX_SHIFT, vi);
	glGetIntegerv(GL_INDEX_OFFSET, vi);

	glGetFloatv(GL_RED_SCALE, v);
	CHKERROR(FLOATEQ(v[0], P->redScale), "redScale");
	glGetFloatv(GL_RED_BIAS, v);
	CHKERROR(FLOATEQ(v[0], P->redBias), "redBias");
	glGetFloatv(GL_GREEN_SCALE, v);
	CHKERROR(FLOATEQ(v[0], P->greenScale), "greenScale");
	glGetFloatv(GL_GREEN_BIAS, v);
	CHKERROR(FLOATEQ(v[0], P->greenBias), "greenBias");
	glGetFloatv(GL_BLUE_SCALE, v);
	CHKERROR(FLOATEQ(v[0], P->blueScale), "blueScale");
	glGetFloatv(GL_BLUE_BIAS, v);
	CHKERROR(FLOATEQ(v[0], P->blueBias), "blueBias");
	glGetFloatv(GL_ALPHA_SCALE, v);
	CHKERROR(FLOATEQ(v[0], P->alphaScale), "alphaScale");
	glGetFloatv(GL_ALPHA_BIAS, v);
	CHKERROR(FLOATEQ(v[0], P->alphaBias), "alphaBias");
	glGetFloatv(GL_DEPTH_SCALE, v);
	CHKERROR(FLOATEQ(v[0], P->depthScale), "depthScale");
	glGetFloatv(GL_DEPTH_BIAS, v);
	CHKERROR(FLOATEQ(v[0], P->depthBias), "depthBias");
	glGetFloatv(GL_ZOOM_X, v);
	CHKERROR(FLOATEQ(v[0], P->xZoom), "xZoom");
	glGetFloatv(GL_ZOOM_Y, v);
	CHKERROR(FLOATEQ(v[0], P->yZoom), "yZoom");
	glGetIntegerv(GL_READ_BUFFER, vi);
	CHKERROR(vi[0] == P->readBuffer, "readBuffer");
    }
    if (bits & GL_POINT_BIT) {
	glGetFloatv(GL_POINT_SIZE, v);
	CHKERROR(FLOATEQ(v[0], GSP->point.smoothSize), "smoothSize");
    }
    if (bits & GL_POLYGON_BIT) {
	_GLpolygonState *P = &GSP->polygon;
	glGetIntegerv(GL_CULL_FACE_MODE, vi);
	CHKERROR(vi[0] == P->cullMode, "cullMode");
	glGetIntegerv(GL_FRONT_FACE, vi);
	CHKERROR(vi[0] == P->frontFaceDirection, "frontFaceDirection");
	glGetIntegerv(GL_POLYGON_MODE, vi);
	CHKERROR(vi[0] == P->frontMode, "frontMode");
	CHKERROR(vi[1] == P->backMode, "backMode");
    }
    if (bits & GL_POLYGON_STIPPLE_BIT) {
	GLubyte *P = GSP->polystipple;
	GLubyte ps[32*4];
        int noError;

	glGetPolygonStipple(ps);
	for (i = 0; i < 32*4; i++) {
	    noError = ps[i] == P[i];
	    CHKERROR(noError, "Polygon stipple pattern");
            if (!noError) {
                GLuint arr[32], arr1[32];

                bcopy(ps, arr, 32*4);
                bcopy(P, arr1, 32*4);
                /* We can clobber i as we are about to break from the loop */
                for (i = 0; i < 32; i += 4) {
                    ogEnvLog(OG_LALWAYS,
                             "Correct patt [%d] 0x%x 0x%x 0x%x 0x%x\n",
                             i, arr1[i], arr1[i+1], arr1[i + 2], arr1[i+3]);
                    ogEnvLog(OG_LALWAYS,
                             "Return  patt [%d] 0x%x 0x%x 0x%x 0x%x\n",
                             i, arr[i], arr[i+1], arr[i + 2], arr[i+3]);
                }
                break;
            }
	}
    }
    if (bits & GL_SCISSOR_BIT) {
	glGetIntegerv(GL_SCISSOR_BOX, vi);
	CHKERROR(EQ4(vi, &GSP->scissor.x), "scissor");
    }
    if (bits & GL_STENCIL_BUFFER_BIT) {
        /*XXX more needed! */
	glGetIntegerv(GL_STENCIL_WRITEMASK, vi);
	/* only consider bits that are active for this visual */
	vi[0] &= (1 << ogEnvCurVisualInfo(GLX_STENCIL_SIZE)) - 1;
        CHKERROR(vi[0] == GSP->stencil.stencilMask, "stencilMask");
    }
    if (bits & GL_TEXTURE_BIT) {
	/* XXX still much checking is missing! (most texture state) */
	_GLtextureState *T = &GSP->texture;

        CHK_ENABLE(TEXTURE_2D, texture2d);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, vi);
        CHKERROR(vi[0] == T->minf2d, "texture2D minfilter");
	glGetTexParameterfv(GL_TEXTURE_2D , GL_TEXTURE_BORDER_COLOR, v);
        CHKERROR(FLOATEQ4(v, (float *) &T->border), "border color");
        glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, vi);
        CHKERROR(vi[0] == T->env.mode, "texenv mode");
        glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, v);
        CHKERROR(FLOATEQ4(v, (float *) &T->env.color), "texenv color");
        glGetTexGeniv(GL_S, GL_TEXTURE_GEN_MODE, vi);
        CHKERROR(vi[0] == T->s.mode, "s texgen mode");
        glGetTexGeniv(GL_T, GL_TEXTURE_GEN_MODE, vi);
        CHKERROR(vi[0] == T->t.mode, "t texgen mode");
        glGetTexGeniv(GL_R, GL_TEXTURE_GEN_MODE, vi);
        CHKERROR(vi[0] == T->r.mode, "r texgen mode");
        glGetTexGeniv(GL_Q, GL_TEXTURE_GEN_MODE, vi);
        CHKERROR(vi[0] == T->q.mode, "q texgen mode");
    }
    if (bits & GL_TRANSFORM_BIT) {
	_GLtransformState *T = &GSP->transform;
	CHK_ENABLE(NORMALIZE, normalize);
        glGetIntegerv(GL_MATRIX_MODE, vi);
	CHKERROR(vi[0] == T->matrixMode, "matrixMode");
	for (i = 0; i < _GL_NUM_CLIP_PLANES; i++) {
	    glGetClipPlane(GL_CLIP_PLANE0+i, vd);
            if (!DOUBLEEQ4(vd, T->clipPlanes[i]))
                ogEnvLog(OG_LFAIL,
                         "TRANSFORM(%s): clipPlane[%d] (%lg %lg %lg %lg) != \
(%lg %lg %lg %lg)\n", stateNames[state], i, vd[0], vd[1], vd[2], vd[3],
                         T->clipPlanes[i][0], T->clipPlanes[i][1],
                         T->clipPlanes[i][2], T->clipPlanes[i][3]);
	}
    }
    if (bits & GL_VIEWPORT_BIT) {
	_GLviewportState *V = &GSP->viewport;
	glGetIntegerv(GL_VIEWPORT, vi);
	CHKERROR(EQ4(vi, &V->x), "viewport");
	glGetDoublev(GL_DEPTH_RANGE, vd);
	CHKERROR(DOUBLEEQ2(vd, &V->zNear), "depthrange");
    }
#   undef CHKERROR
#   undef CHK_ENABLE
}

static void
expecterror(GLenum code, char *msg)
{
    GLenum err;
    if (code != (err = glGetError()))
	ogEnvLog(OG_LFAIL, "%s: got error %s (expected %s)\n",
                 msg, gluErrorString(err), gluErrorString(code));
}


/*
 *  define an opengl state structure and fill it with some reasonable
 *  values.  start by copying the default state.
 */
static void
initalternatestate(void)
{
    int i;

    SAlternate = SDefault;
    /* accum buffer */
    {
    _GLaccumState *A = &SAlternate.accum;
    SETRGBA(A->aclear, .1, .2, .3, .4);
    }
    /* color buffer */
    {
    _GLrasterState *R = &SAlternate.raster;

    R->alphaFunc = GL_ALWAYS; /*default*/
    R->alphaRef = 0.0; /*default*/
    R->blendSrc = GL_ONE; /*default*/
    R->blendDst = GL_ZERO; /*default*/
    R->logicOp = GL_XOR;
    R->drawBuffer = GL_NONE;
    R->cclear.r = 0.5;
    R->cclear.g = 0.8;
    R->cclear.b = 0.33;
    R->cclear.a = 0.76;
    R->clearIndex = 0.7; 
    R->writeMask = (0xffffffff >> (32-buffersize)) & 0x55555555;
    R->rgbaMask[0] = R->rgbaMask[1] = R->rgbaMask[3] = GL_FALSE;
    R->rgbaMask[2] = GL_TRUE;
    R->cclear.r = .1, R->cclear.g = .2, R->cclear.b = .3, R->cclear.a = .4;
    }
    /* current */
    {
    _GLcurrentState *C = &SAlternate.current;
    SETRGBA(C->color, .1, .2, .3, .4);
    C->index = 2;
    SETSTRQ(C->texCoord, .5, .7, .9, -1);
    SETXYZW(C->normal, .7071, .7071, 0, 1);
    SETXYZW(C->rasterPos, .2, .4, .6, .8);
    C->rasterZ = 0; /* default */
    /* for rgba visuals raster index won't change and vice versa */
    /* Raster color or index need to be the same as the setting above */
    if (rgbamode) {
	SETRGBA(C->rasterColor, .1, .2, .3, .4);
    } else {
	C->rasterIndex = 2;
    }
    SETSTRQ(C->rasterTexCoord, .5, .7, .9, -1); /* make same as texCoord */
    C->validRasterPos = 1; /* default */
    C->edgeFlag = GL_FALSE;
    }
    /* depth buffer */
    {
    _GLdepthState *D = &SAlternate.depth;
    D->depthFunc = GL_ALWAYS;
    D->writeEnable = 0;
    D->zclear = .5;
    }
    /* enable */
    {
    _GLenableState *E = &SAlternate.enables;
    E->alpha_test = 1;
    E->auto_normal = 1;
    E->blend = 1;
    E->color_material = 1;
    E->cull_face = 1;
    E->depth_test = 1;
    E->dither = 0;
    E->fog = 1;
    E->lighting = 1;
    E->line_smooth = 1;
    E->line_stipple = 1;
    E->logic_op = 1;
    E->normalize = 1;
    E->point_smooth = 1;
    E->polygon_smooth = 1;
    E->polygon_stipple = 1;
    E->scissor_test = 1;
    E->stencil_test = 1;
    E->texture1d = 0;
    E->texture2d = 1;
    E->texture3d = 0;
    E->tex_gen_s = 1;
    E->tex_gen_t = 1;
    E->tex_gen_r = 1;
    E->tex_gen_q = 1;

    /* turn on lights 0, 2, 4, 6, and 7 */
    for (i = 0; i < _GL_NUM_LIGHTS; i++)
	E->lights[i] = (i&1) ? 0 : 1;
    E->lights[7] = 1;
    for(i = 0; i < _GL_NUM_CLIP_PLANES; i++)
	E->clipPlanes[i] = 1;

    E->eval1d.color4 = E->eval2d.color4 = 1;
    E->eval1d.index = E->eval2d.index = 1;
    E->eval1d.normal = E->eval2d.normal = 1;
    E->eval1d.textureCoord1 = E->eval2d.textureCoord1 = 1;
    E->eval1d.textureCoord2 = E->eval2d.textureCoord2 = 1;
    E->eval1d.textureCoord3 = E->eval2d.textureCoord3 = 0;
    E->eval1d.textureCoord4 = E->eval2d.textureCoord4 = 1;
    E->eval1d.vertex3 = E->eval2d.vertex3 = 1;
    E->eval1d.vertex4 = E->eval2d.vertex4 = 1;
    
    /* extensions */
    E->convolution1d = 0;
    E->convolution2d = 1;
    E->histogram = 1;
    E->minmax = 1;
    E->polygon_offset = 1;
    E->separable2d = 0;

    E->MSsampleMask = GL_TRUE;
    E->MSalphaToOne = GL_FALSE;
    E->MSalphaToMask = GL_TRUE;
    }
    /* evaluators */
    {
    SAlternate.evals.segments1d = 4;
    SAlternate.evals.domain1d[0] = 0.5;
    SAlternate.evals.domain1d[1] = 5;
    SAlternate.evals.segments2d[0] = 8;
    SAlternate.evals.domain2d[0][0] = 1;
    SAlternate.evals.domain2d[0][1] = 6;
    SAlternate.evals.segments2d[1] = 5;
    SAlternate.evals.domain2d[1][0] = 3;
    SAlternate.evals.domain2d[1][1] = 7;
    }
    /* fog */
    {
    _GLfogState *F = &SAlternate.fog;
    F->mode = GL_LINEAR;
    F->color.r = .1; F->color.g = .2; F->color.b = .3; F->color.a = .4;
    F->density = .5;
    F->start = .3;
    F->end = .9;
    F->index = 2;
    }
    /* hint */
    {
    _GLhintState *H = &SAlternate.hints;
    H->perspectiveCorrection = GL_NICEST;
    H->pointSmooth = GL_NICEST;
    H->lineSmooth = GL_NICEST;
    H->polygonSmooth = GL_NICEST;
    H->fog = GL_NICEST;
    }
    /* lighting */
    {
    _GLmaterialState *F = &SAlternate.light.front;
    _GLmaterialState *B = &SAlternate.light.back;

    /* XXX light state still missing here! */
    SETRGBA(F->ambient, .5, .6, .7, .8);
    SETRGBA(F->diffuse, .11, .22, .33, .44);
    SETRGBA(F->specular, .2, .3, .4, .5);
    SETRGBA(F->emission, .15, .25, .35, .45);
    F->shininess = 99;
    F->cmap[0] = .2;
    F->cmap[1] = .3;
    F->cmap[2] = .6;

    SETRGBA(B->ambient, .8, .7, .6, .5);
    SETRGBA(B->diffuse, .44, .33, .22, .11);
    SETRGBA(B->specular, .5, .4, .3, .2);
    SETRGBA(B->emission, .45, .35, .25, .15);
    B->shininess = 66;
    B->cmap[0] = .6;
    B->cmap[1] = .4;
    B->cmap[2] = .2;
    }
    /* line */
    {
    _GLlineState *L = &SAlternate.line;
    L->smoothWidth = 2.0;
    /***L->aliasedWidth = 1;***/
    L->stipple = 0x5995;
    L->stippleRepeat = 3;
    }
    /* list */
    {
    _GLlistState *L = &SAlternate.list;
    L->listBase = 999;
    }
    /* pixel mode */
    {
    _GLpixelState *P = &SAlternate.pixel;
    P->mapColor = GL_TRUE;
    P->mapStencil = GL_TRUE;
    P->indexShift = -1;
    P->indexOffset = .5;
    P->redScale = .33;
    P->redBias = 3;
    P->greenScale = .44;
    P->greenBias = 4;
    P->blueScale = .55;
    P->blueBias = 5;
    P->alphaScale = .66;
    P->alphaBias = 6;
    P->depthScale = .7;
    P->depthBias = 7;
    P->xZoom = 2;
    P->yZoom = 3;
    P->readBuffer = GL_FRONT; /* the only choice on some visuals */
    }
    
    /* point */
    {
    _GLpointState *P = &SAlternate.point;
    P->smoothSize = 2.0;
    }
    /* polygon */
    {
    _GLpolygonState *P = &SAlternate.polygon;
    P->cullMode = GL_FRONT;
    P->frontFaceDirection = GL_CW;
    P->frontMode = GL_POINT;
    P->backMode = GL_LINE;
    }
    /* polygon stipple */
    {
    GLubyte *P = SAlternate.polystipple;
    int i;
    for (i = 0; i < 32*4; i++) P[i] = ogLibBitRand(8);
    }
    /* scissor */
    {
    _GLscissorState *S = &SAlternate.scissor;
    S->x = 21;
    S->y = 22;
    S->width = 23;
    S->height = 24;
    }
    
    /* stencil buffer */
    {
    _GLstencilState *S = &SAlternate.stencil;
    S->testFunc = GL_ALWAYS;	
    S->funcRef = 0;
    S->funcMask = 0xff;
    S->stencilMask = 0x0;
    S->fail = GL_KEEP;
    S->pass = GL_KEEP;
    S->depthPass = GL_KEEP;
    S->sclear = 0;
    }
    /* texture */
    {
    _GLtextureState *T = &SAlternate.texture;

    T->minf2d = GL_LINEAR;
    /* SGIS_texture_select */
    T->dual_texsel = 1;
    T->quad_texsel = 3;
    /* SGIX_texture_scale_bias */
    T->tsb_scales[0] = 0.1;
    T->tsb_scales[1] = 0.2;
    T->tsb_scales[2] = 0.3;
    T->tsb_scales[3] = 0.4;
    T->tsb_biases[0] = 0.5;
    T->tsb_biases[1] = 0.6;
    T->tsb_biases[2] = 0.7;
    T->tsb_biases[3] = 0.8;
    T->border.r = 0.5;
    T->border.g = 0;
    T->border.b = 1;
    T->border.a = 0.1;
    T->env.mode = GL_BLEND;
    T->env.color.r = 0.4;
    T->env.color.g = 0.6;
    T->env.color.b = 0.4;
    T->env.color.a = 0.1;
    T->env.bias.r = -0.4;
    T->env.bias.g = 0.1;
    T->env.bias.b = -0.8;
    T->env.bias.a = 0.1;
    
    T->s.mode = GL_OBJECT_LINEAR;
    T->t.mode = GL_SPHERE_MAP;
    T->r.mode = GL_EYE_LINEAR;
    T->q.mode = GL_OBJECT_LINEAR;
    /* TEXGEN plane equations are missing */
    }
    /* transform */
    {
    _GLtransformState *T = &SAlternate.transform;
    int i;

    T->matrixMode = GL_TEXTURE;
    for(i = 0; i < _GL_NUM_CLIP_PLANES; i++) {
	T->clipPlanes[i][0] = ogLibFloatRand(-10, 10);
	T->clipPlanes[i][1] = ogLibFloatRand(-10, 10);
	T->clipPlanes[i][2] = ogLibFloatRand(-10, 10);
	T->clipPlanes[i][3] = ogLibFloatRand(-10, 10);
    }
    }
    /* viewport */
    {
    _GLviewportState *V = &SAlternate.viewport;

    V->x = 11;
    V->y = 12;
    V->width = 13;
    V->height = 14;
    V->zNear = .25;
    V->zFar = .75;
    }
}

/*
 *  define a DEFAULT opengl state structure.
 */
static void
initdefaultstate(void)
{
    int i;

    /* XXX opengl - much more init'ing needed! */

    /* accum buffer */
    {
    _GLaccumState *A = &SDefault.accum;
    A->aclear.r = A->aclear.g = A->aclear.b = A->aclear.a = 0.0;
    }
    /* color buffer */
    {
    _GLrasterState *R = &SDefault.raster;

    R->alphaFunc = GL_ALWAYS;
    R->alphaRef = 0;
    R->blendSrc = GL_ONE;
    R->blendDst = GL_ZERO;
    R->logicOp = GL_COPY;
    R->drawBuffer = ogEnvCurVisualInfo(GLX_DOUBLEBUFFER) ? GL_BACK : GL_FRONT;
    R->cclear.r = R->cclear.g = R->cclear.b = R->cclear.a = 0.0;
    R->clearIndex = 0.0;
    R->writeMask = 0xffffffff >> (32-buffersize);
    R->rgbaMask[0] = R->rgbaMask[1] = R->rgbaMask[2] = R->rgbaMask[3] = 1;
    }
    /* current */
    {
    _GLcurrentState *C = &SDefault.current;
    if (ogEnvIsDualPersonality() && rgbamode) {
	SETRGBA(C->color, 1, 0, 0, 1);
    } else {
	SETRGBA(C->color, 1, 1, 1, 1);
    }
    C->index = 1;
    SETSTRQ(C->texCoord, 0, 0, 0, 1);
    SETXYZW(C->normal, 0, 0, 1, 1);
    SETXYZW(C->rasterPos, 0, 0, 0, 1);
    C->rasterZ = 0;
    SETRGBA(C->rasterColor, 1, 1, 1, 1);
    C->rasterIndex = 1;
    SETSTRQ(C->rasterTexCoord, 0, 0, 0, 1);
    C->validRasterPos = 1;
    C->edgeFlag = GL_TRUE;
    }
    /* depth buffer */
    {
    _GLdepthState *D = &SDefault.depth;
    D->depthFunc = GL_LESS;
    D->writeEnable = 1;
    D->zclear = 1.0;
    }
    /* enable */
    {
    _GLenableState *E = &SDefault.enables;
    bzero(E, sizeof(_GLenableState));
    E->dither = 1;		/* the only enable that's TRUE by default */
    }
    /* evaluators */
    {
    SDefault.evals.domain1d[0] = 0;
    SDefault.evals.domain1d[1] = 1;
    SDefault.evals.segments1d = 1;
    SDefault.evals.domain2d[0][0] = SDefault.evals.domain2d[1][0] = 0;
    SDefault.evals.domain2d[0][1] = SDefault.evals.domain2d[1][1] = 1;
    SDefault.evals.segments2d[0] = SDefault.evals.segments2d[1] = 1;
    }
    /* fog */
    {
    _GLfogState *F = &SDefault.fog;
    F->mode = GL_EXP;
    F->color.r = F->color.g = F->color.b = F->color.a = 0.;
    F->density = 1.0;
    F->start = 0.;
    F->end = 1.0;
    F->index = 0.;
    }
    /* hint */
    {
    _GLhintState *H = &SDefault.hints;
    H->perspectiveCorrection = GL_DONT_CARE;
    H->pointSmooth = GL_DONT_CARE;
    H->lineSmooth = GL_DONT_CARE;
    H->polygonSmooth = GL_DONT_CARE;
    H->fog = GL_DONT_CARE;
    }
    /* lighting */
    {
    _GLmaterialState *matp;
    _GLlightModelState *lmp;
    _GLlightSourceState *sources;

    SDefault.light.colorMaterialFace = GL_FRONT_AND_BACK;
    SDefault.light.colorMaterialMode = GL_AMBIENT_AND_DIFFUSE;
    SDefault.light.shadeModel = GL_SMOOTH;
    /* default material - do both front and back */
    matp = &SDefault.light.front;
    for (i = 0; i < 2; i++) {
	SETRGBA(matp->ambient, .2, .2, .2, 1);
	SETRGBA(matp->diffuse, .8, .8, .8, 1);
	SETRGBA(matp->specular, 0, 0, 0, 1);
	SETRGBA(matp->emission, 0, 0, 0, 1);
	matp->shininess = 0;
	matp->cmap[0] = 0; /* a */
	matp->cmap[1] = 1; /* d */
	matp->cmap[2] = 1; /* s */

	matp = &SDefault.light.back;
    }
    /* default light model */
    lmp = &SDefault.light.model;
    SETRGBA(lmp->ambient, .2, .2, .2, 1);
    lmp->localViewer = 0;
    lmp->twoSide = 0;
    /* default lights */
    sources = SDefault.light.sources;
    for (i = 0; i < _GL_NUM_LIGHTS; i++) {
	SETRGBA(sources[i].ambient, 0, 0, 0, 1);
	if (i) {
	    SETRGBA(sources[i].diffuse, 0, 0, 0, 1);
	    SETRGBA(sources[i].specular, 0, 0, 0, 1);
	} else {
	    SETRGBA(sources[i].diffuse, 1, 1, 1, 1);
	    SETRGBA(sources[i].specular, 1, 1, 1, 1);
	}
	SETXYZW(sources[i].position, 0, 0, 1, 0);
	SETXYZW(sources[i].spotDirection, 0, 0, -1, 0);

	sources[i].userpositionflag = 1; /* set.. to force xform */
	sources[i].userspotdirflag = 1;

	sources[i].spotLightExponent = 0;
	sources[i].spotLightCutOffAngle = 180;
	sources[i].constantAttenuation = 1;
	sources[i].linearAttenuation = 0;
	sources[i].quadraticAttenuation = 0;
    }
    }
    /* line */
    {
    _GLlineState *L = &SDefault.line;
    L->smoothWidth = 1.0;
    L->aliasedWidth = 1;
    L->stipple = 0xffff;
    L->stippleRepeat = 1;
    }
    /* list */
    {
    _GLlistState *L = &SDefault.list;
    L->listBase = 0;
    }
    /* pixel mode */
    {
    _GLpixelState *P = &SDefault.pixel;
    P->mapColor = GL_FALSE;
    P->mapStencil = GL_FALSE;
    P->indexShift = 0;
    P->indexOffset = 0;
    P->redScale = 1;
    P->redBias = 0;
    P->greenScale = 1;
    P->greenBias = 0;
    P->blueScale = 1;
    P->blueBias = 0;
    P->alphaScale = 1;
    P->alphaBias = 0;
    P->depthScale = 1;
    P->depthBias = 0;
    P->xZoom = 1;
    P->yZoom = 1;
    P->readBuffer = ogEnvCurVisualInfo(GLX_DOUBLEBUFFER) ? GL_BACK : GL_FRONT;
    }
    
    /* point */
    {
    _GLpointState *P = &SDefault.point;
    P->smoothSize = 1.0;
    }
    /* polygon */
    {
    _GLpolygonState *P = &SDefault.polygon;
    P->cullMode = GL_BACK;
    P->frontFaceDirection = GL_CCW;
    P->frontMode = P->backMode = GL_FILL;
    }
    /*
     * polygon stipple
     * If the pattern is all 1's, it is possible to hopelessly garble
     * it and still have the test pass, therefore we do not use the default
     * pattern but a more interesting one.
     */
    {
    GLubyte *P = SDefault.polystipple;
    for (i = 0; i < 32*4; i++)
        P[i] = ogLibBitRand(8);
    glPolygonStipple(P);
    }
    /* scissor */
    {
    _GLscissorState *S = &SDefault.scissor;
    S->x = 0;
    S->y = 0;
    S->width = ogEnvQuery(OG_XWSIZE);
    S->height = ogEnvQuery(OG_YWSIZE);
    }
    
    /* stencil buffer */
    {
    _GLstencilState *S = &SDefault.stencil;
    S->testFunc = GL_ALWAYS;	
    S->funcRef = 0;
    S->funcMask = 0xff;
    S->stencilMask = (1 << ogEnvCurVisualInfo(GLX_STENCIL_SIZE)) - 1;
    S->fail = GL_KEEP;
    S->pass = GL_KEEP;
    S->depthPass = GL_KEEP;
    S->sclear = 0;
    }
    /* texture */
    {
    _GLtextureState *T = &SDefault.texture;

    T->minf2d = GL_NEAREST_MIPMAP_LINEAR;
    /* SGIS_texture_select */
    T->dual_texsel = T->quad_texsel = 0;
    /* SGIX_texture_scale_bias */
    T->tsb_scales[0] = T->tsb_scales[1] = T->tsb_scales[2] =
        T->tsb_scales[3] = 1.0;
    T->tsb_biases[0] = T->tsb_biases[1] = T->tsb_biases[2] = 
        T->tsb_biases[3] = 0.0;
    T->border.r = T->border.g = T->border.b = T->border.a = 0;
    T->env.mode = GL_MODULATE;
    T->env.color = T->env.bias = T->env.color;
    
    T->s.mode = T->r.mode = T->t.mode = T->q.mode = GL_EYE_LINEAR;;
    /* TEXGEN is missing */
    }
    /* transform */
    {
    _GLtransformState *T = &SDefault.transform;
    int i;

    T->matrixMode = GL_MODELVIEW;
    for(i = 0; i < _GL_NUM_CLIP_PLANES; i++) {
	T->clipPlanes[i][0] = 
	T->clipPlanes[i][1] = 
	T->clipPlanes[i][2] = 
	T->clipPlanes[i][3] = 0;
    }
    }
    /* viewport */
    {
    _GLviewportState *V = &SDefault.viewport;
    V->x = 0.0;
    V->y = 0.0;
    V->width = ogEnvQuery(OG_XWSIZE);
    V->height = ogEnvQuery(OG_YWSIZE);
    V->zNear = 0.0;
    V->zFar = 1.0;
    }

#if 0 /* XXX do these someday! */
    /* Multisampling */
    SDefault.multiSampling.mask = 1;
    SDefault.multiSampling.alphaMode = MSA_MASK_ONE;

    /* convolution */
    {
        FilterSpecOGL *f;

        f = &SDefault.Convolve.filter2d;
	f->kernel[0] =
	f->kernel[1] = 
	f->kernel[2] = 
	f->kernel[3] =
	f->kernel[5] =
	f->kernel[6] =
	f->kernel[7] =
	f->kernel[8] = 0.0;
	f->kernel[4] = 1.0;

	f->internalformat = GL_LUMINANCE;
	f->xksize =
	f->yksize = 3;
	f->enable = 0;

	f = &SDefault.Convolve.filterSeparable2d;
        f->kernel[0] =
	f->kernel[1] = 
	f->kernel[3] = 
	f->kernel[4] =
	f->kernel[5] =
	f->kernel[6] =
	f->kernel[8] =
	f->kernel[9] = 0.0;
	f->kernel[7] =
	f->kernel[2] = 1.0;

	f->internalformat = GL_LUMINANCE;
	f->xksize =
	f->yksize = 3;
	f->enable = 0;
    }
    /* histogram */
    SDefault.Histogram.sink = 1;
    SDefault.Histogram.enable = 0;
    
    /* minmax */
    SDefault.minmax.sink = 1;
    SDefault.minmax.enable = 0;
#endif
}

static char *
attrib2str(GLenum bits)
{
    static char buf[80];
#define BIT_TO_NAME(bit) case GL_##bit##_BIT: return #bit "_BIT"
    switch (bits) {
      case GL_ALL_ATTRIB_BITS:
        return "GL_ALL_ATTRIB_BITS";
      BIT_TO_NAME(ACCUM_BUFFER);
      BIT_TO_NAME(COLOR_BUFFER);
      BIT_TO_NAME(CURRENT);
      BIT_TO_NAME(DEPTH_BUFFER);
      BIT_TO_NAME(ENABLE);
      BIT_TO_NAME(EVAL);
      BIT_TO_NAME(FOG);
      BIT_TO_NAME(HINT);
      BIT_TO_NAME(LIGHTING);
      BIT_TO_NAME(LINE);
      BIT_TO_NAME(LIST);
      BIT_TO_NAME(PIXEL_MODE);
      BIT_TO_NAME(POINT);
      BIT_TO_NAME(POLYGON);
      BIT_TO_NAME(POLYGON_STIPPLE);
      BIT_TO_NAME(SCISSOR);
      BIT_TO_NAME(STENCIL_BUFFER);
      BIT_TO_NAME(TEXTURE);
      BIT_TO_NAME(TRANSFORM);
      BIT_TO_NAME(VIEWPORT);
      default:
        sprintf(buf, "bits 0x%x", bits);
        return buf;
    }
}

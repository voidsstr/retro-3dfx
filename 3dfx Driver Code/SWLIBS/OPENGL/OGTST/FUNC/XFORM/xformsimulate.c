/**************************************************************************
 *									  *
 * 		 Copyright (C) 1989, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/* simulate.c - $Revision: 2$ */

#include "ogtst.h"
#include "xform.h"
#include <math.h>

#include "stdio.h"		/* XXXblythe */

static int obj = 0;
static int pickmode = 0;
static int GtstMmode = GL_MODELVIEW;

static float View[4][4] = {1.0, 0.0, 0.0, 0.0,
		           0.0, 1.0, 0.0, 0.0,
		           0.0, 0.0, 1.0, 0.0,
		           0.0, 0.0, 0.0, 1.0};

static float Prsp[4][4] = {1.0, 0.0, 0.0, 0.0,
		          0.0, 1.0, 0.0, 0.0,
		          0.0, 0.0, 1.0, 0.0,
		          0.0, 0.0, 0.0, 1.0};

static float Mtex[4][4] = {1.0, 0.0, 0.0, 0.0,
		           0.0, 1.0, 0.0, 0.0,
		           0.0, 0.0, 1.0, 0.0,
		           0.0, 0.0, 0.0, 1.0};

static float Comp[4][4] = {1.0, 0.0, 0.0, 0.0,
		           0.0, 1.0, 0.0, 0.0,
		           0.0, 0.0, 1.0, 0.0,
		           0.0, 0.0, 0.0, 1.0};

static float Pick[4][4] = {1.0, 0.0, 0.0, 0.0,
		           0.0, 1.0, 0.0, 0.0,
		           0.0, 0.0, 1.0, 0.0,
		           0.0, 0.0, 0.0, 1.0};

static void 
tx_frotmat(float, char, float[4][4]);

/*************************************************************
*  tx_frotmat()  -  construct a rotation matrix given
*                   (theta,axis) for the GL rot command
*************************************************************/
static void 
tx_frotmat(float theta, char axis, float mat[4][4])
{
    float sine, cosi;

    theta *= M_PI / 180.0;
    while (theta < -M_PI)
	theta += 2.0 * M_PI;
    while (theta > M_PI)
	theta -= 2.0 * M_PI;

    ogLibUntMatrix(mat[0]);
    sine = sin(theta);
    cosi = cos(theta);

    switch (axis) {
    case 'x':
    case 'X':
	mat[1][1] = cosi;
	mat[1][2] = sine;
	mat[2][1] = -sine;
	mat[2][2] = cosi;
	break;

    case 'y':
    case 'Y':
	mat[0][0] = cosi;
	mat[0][2] = -sine;
	mat[2][0] = sine;
	mat[2][2] = cosi;
	break;

    case 'z':
    case 'Z':
	mat[0][0] = cosi;
	mat[0][1] = sine;
	mat[1][0] = -sine;
	mat[1][1] = cosi;
	break;
    }
}

static void 
tx_rotmat(float angle, float ax, float ay, float az, float mat[4][4])
{
    GLfloat radians, sine, cosine, ab, bc, ca, t;

    t = sqrt(ax * ax + ay * ay + az * az);
    ax /= t;
    ay /= t;
    az /= t;

    radians = angle * M_PI / 180.0;

    sine = sin(radians);
    cosine = cos(radians);
    ab = ax * ay * (1 - cosine);
    bc = ay * az * (1 - cosine);
    ca = az * ax * (1 - cosine);

    ogLibUntMatrix(mat[0]);
    t = ax * ax;
    mat[0][0] = t + cosine * (1 - t);
    mat[2][1] = bc - ax * sine;
    mat[1][2] = bc + ax * sine;

    t = ay * ay;
    mat[1][1] = t + cosine * (1 - t);
    mat[2][0] = ca + ay * sine;
    mat[0][2] = ca - ay * sine;

    t = az * az;
    mat[2][2] = t + cosine * (1 - t);
    mat[1][0] = ab - az * sine;
    mat[0][1] = ab + az * sine;
}

/*************************************************************
*  gtst_init_matrixstuff()  - 
*************************************************************/
void 
gtst_init_matrixstuff(void)
{
    GtstMmode = GL_MODELVIEW;
    glMatrixMode(GL_MODELVIEW);

    ogLibUntMatrix(View[0]);
    ogLibUntMatrix(Prsp[0]);
    ogLibUntMatrix(Comp[0]);
    ogLibUntMatrix(Pick[0]);
    ogLibUntMatrix(Mtex[0]);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((GLfloat *) Comp);
    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf((GLfloat *) Mtex);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf((GLfloat *) Comp);
}

/*************************************************************
*  gtst_mmode()  - 
*************************************************************/
void 
gtst_mmode(int mode)
{

    GtstMmode = mode;

    START_DL_OR_IM(1)
	glMatrixMode(mode);
    switch (mode) {
    case GL_MODELVIEW:
	ogEnvLog(1, "glMatrixMode(GL_MODELVIEW);\n");
	break;
    case GL_PROJECTION:
	ogEnvLog(1, "glMatrixMode(GL_PROJECTION);\n");
	break;
    case GL_TEXTURE:
	ogEnvLog(1, "glMatrixMode(GL_TEXTURE);\n");
	break;
    default:
	ogEnvLog(1, "glMatrixMode(ERROR);\n");
	break;
    }
    FINIS_DL_OR_IM(1)
}

/*************************************************************
*  gtst_getmmode()  - 
*************************************************************/
int 
gtst_getmmode(void)
{
    GLint a;

    glGetIntegerv(GL_MATRIX_MODE, &a);
    ogEnvLog(1, "%d = glGetIntegerv(GL_MATRIX_MODE);\n", a);

    if (GtstMmode != a)
	ogEnvLog(OG_LFAIL, "matrix mode does not match simulation (sim) 0x%x (pipe) 0x%x\n", GtstMmode, a);

    return a;
}

/*************************************************************
*  gtst_loadmatrix()  - 
*************************************************************/
void 
gtst_loadmatrix(float *m)
{

    if (GtstMmode == GL_MODELVIEW) {
	ogLibCpyMatrix(View[0], m);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_PROJECTION) {
	ogLibCpyMatrix(Prsp[0], m);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_TEXTURE)
	ogLibCpyMatrix(Mtex[0], m);
    else
	ogEnvLog(OG_LINTERNALERROR, "gtst_loadmatrix()\n");

    START_DL_OR_IM(1)
	glLoadMatrixf((GLfloat *) m);
    ogEnvLog(1, "glLoadMatrixf(0x%x);\n", m);
    FINIS_DL_OR_IM(1)
}

/*************************************************************
*  gtst_getmatrix()  - 
*************************************************************/
void 
gtst_getmatrix(float *m, GLint flag)
{
    float simul[4][4];

    if (GtstMmode == GL_MODELVIEW) {
	ogLibCpyMatrix(simul[0], View[0]);
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) m);
	ogEnvLog(1, "glGetFloatv(GL_MODELVIEW_MATRIX, 0x%x)\n", m);
    } else if (GtstMmode == GL_PROJECTION) {
	if (pickmode)
	    ogLibMulMatrix(simul[0], Prsp[0], Pick[0]);
	else
	    ogLibCpyMatrix(simul[0], Prsp[0]);
	glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *) m);
	ogEnvLog(1, "glGetFloatv(GL_PROJECTION_MATRIX, 0x%x)\n", m);
    } else if (GtstMmode == GL_TEXTURE) {
	ogLibCpyMatrix(simul[0], Mtex[0]);
	glGetFloatv(GL_TEXTURE_MATRIX, (GLfloat *) m);
	ogEnvLog(1, "glGetFloatv(GL_TEXTURE_MATRIX, 0x%x)\n", m);
    } else
	ogEnvLog(OG_LINTERNALERROR, "gtst_getmatrix()\n");


    ogLibPntMatrix(m, 3, "real pipe");
    ogLibPntMatrix(simul[0], 3, "simulation");

    if (flag)
	ogLibRcmMatrix(simul[0], m);
    else
	ogLibAcmMatrix(simul[0], m);
}

/*************************************************************
*  gtst_multmatrix()  - 
*************************************************************/
void 
gtst_multmatrix(float *m)
{
    if (GtstMmode == GL_MODELVIEW) {
	ogLibMulMatrix(View[0], m, View[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_PROJECTION) {
	ogLibMulMatrix(Prsp[0], m, Prsp[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_TEXTURE)
	ogLibMulMatrix(Mtex[0], m, Mtex[0]);
    else
	ogEnvLog(OG_LINTERNALERROR, "gtst_multmatrix()\n");

    ogLibPntMatrix(m, 3, "multiplication");

    START_DL_OR_IM(1)
	glMultMatrixf((GLfloat *) m);
    ogEnvLog(1, "glMultMatrixf(0x%x);\n", m);
    FINIS_DL_OR_IM(1)
}

/*************************************************************
*  gtst_scale()  - 
*************************************************************/
void 
gtst_scale(float x, float y, float z)
{
    float scal[4][4];

    GLdouble sd[3];

    sd[0] = x;
    sd[1] = y;
    sd[2] = z;

    ogLibUntMatrix(scal[0]);
    scal[0][0] = x;
    scal[1][1] = y;
    scal[2][2] = z;
    ogLibPntMatrix(scal[0], 2, "scaling");

    if (GtstMmode == GL_MODELVIEW) {
	ogLibMulMatrix(View[0], scal[0], View[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_PROJECTION) {
	ogLibMulMatrix(Prsp[0], scal[0], Prsp[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_TEXTURE)
	ogLibMulMatrix(Mtex[0], scal[0], Mtex[0]);
    else
	ogEnvLog(OG_LINTERNALERROR, "gtst_scale()\n");

    START_DL_OR_IM(1)
	if (ogLibBitRand(1)) {
	glScalef(x, y, z);
	ogEnvLog(1, "glScalef(%f, %f, %f);\n", x, y, z);
    } else {
	glScaled(sd[0], sd[1], sd[2]);
	ogEnvLog(1, "glScaled(%f, %f, %f);\n", sd[0], sd[1], sd[2]);
    }
    FINIS_DL_OR_IM(1)
}

/*************************************************************
*  gtst_translate()  - 
*************************************************************/
void 
gtst_translate(float x, float y, float z)
{
    float trans[4][4];
    GLdouble td[3];

    td[0] = x;
    td[1] = y;
    td[2] = z;

    ogLibUntMatrix(trans[0]);
    trans[3][0] = x;
    trans[3][1] = y;
    trans[3][2] = z;
    ogLibPntMatrix(trans[0], 2, "translation");

    if (GtstMmode == GL_MODELVIEW) {
	ogLibMulMatrix(View[0], trans[0], View[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_PROJECTION) {
	ogLibMulMatrix(Prsp[0], trans[0], Prsp[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_TEXTURE)
	ogLibMulMatrix(Mtex[0], trans[0], Mtex[0]);
    else
	ogEnvLog(OG_LINTERNALERROR, "gtst_translate()\n");

    START_DL_OR_IM(1)
	if (ogLibBitRand(1)) {
	glTranslatef(x, y, z);
	ogEnvLog(1, "glTranslatef(%f, %f, %f);\n", x, y, z);
    } else {
	glTranslated(td[0], td[1], td[2]);
	ogEnvLog(1, "glTranslated(%f, %f, %f);\n", td[0], td[1], td[2]);
    }
    FINIS_DL_OR_IM(1)
}

/*************************************************************
*  gtst_rotate()  - 
*************************************************************/
void 
gtst_rotate(float a, float x, float y, float z)
{
    float mat[4][4];

    tx_rotmat(a, x, y, z, mat);
    ogLibPntMatrix(mat[0], 2, "rotation");

    if (GtstMmode == GL_MODELVIEW) {
	ogLibMulMatrix(View[0], mat[0], View[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_PROJECTION) {
	ogLibMulMatrix(Prsp[0], mat[0], Prsp[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_TEXTURE)
	ogLibMulMatrix(Mtex[0], mat[0], Mtex[0]);
    else
	ogEnvLog(OG_LINTERNALERROR, "gtst_rotate()\n");

    START_DL_OR_IM(1)
	if (ogLibBitRand(1)) {
	glRotatef(a, x, y, z);
	ogEnvLog(1, "glRotatef(%f, %f,%f,%f);\n", a, x, y, z);
    } else {
	glRotated(a, x, y, z);
	ogEnvLog(1, "glRotated(%f, %f,%f,%f);\n", a, x, y, z);
    }
    FINIS_DL_OR_IM(1)
}

/*************************************************************
*  gtst_rot()  - 
*************************************************************/
void 
gtst_rot(float theta, char axis)
{
    float mat[4][4];

    tx_frotmat(theta, axis, mat);
    ogLibPntMatrix(mat[0], 2, "rotation");

    if (GtstMmode == GL_MODELVIEW) {
	ogLibMulMatrix(View[0], mat[0], View[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_PROJECTION) {
	ogLibMulMatrix(Prsp[0], mat[0], Prsp[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_TEXTURE)
	ogLibMulMatrix(Mtex[0], mat[0], Mtex[0]);
    else
	ogEnvLog(OG_LINTERNALERROR, "gtst_rot()\n");

    START_DL_OR_IM(1)
    /* OGLXXX You can do better than this. */
	glRotatef(theta, (axis) == 'x', (axis) == 'y', (axis) == 'z');
    /* OGLXXX You can do better than this. */
    ogEnvLog(1, "glRotatef(%f, (%c)=='x', (%c)=='y', (%c)=='z');\n", theta, axis);
    FINIS_DL_OR_IM(1)
}

/*************************************************************
*  gtst_lookat()  - 
*************************************************************/
/*ARGSUSED*/
void 
gtst_lookat(float vx, float vy, float vz, float px, float py, float pz, short twist)
{
#ifdef FIX_XXX
    float a, hypxz, hypxyz, dx, dy, dz;
    float look[4][4], rotx[4][4], roty[4][4], rotz[4][4], trans[4][4];

    dx = px - vx;
    dy = py - vy;
    dz = pz - vz;

    hypxz = sqrt(dx * dx + dz * dz);
    hypxyz = sqrt(dx * dx + dy * dy + dz * dz);

    ogLibUntMatrix(trans);
    trans[3][0] = -vx;
    trans[3][1] = -vy;
    trans[3][2] = -vz;

    ogLibUntMatrix(roty);
    roty[0][0] = -dz / hypxz;
    roty[0][2] = -dx / hypxz;
    roty[2][0] = -roty[0][2];
    roty[2][2] = roty[0][0];

    ogLibUntMatrix(rotx);
    rotx[1][1] = hypxz / hypxyz;
    rotx[1][2] = -dy / hypxyz;
    rotx[2][1] = -rotx[1][2];
    rotx[2][2] = rotx[1][1];

    ogLibUntMatrix(rotz);
    a = -twist * M_PI / 1800.0;
    rotz[0][0] = cos(a);
    rotz[0][1] = sin(a);
    rotz[1][0] = -rotz[0][1];
    rotz[1][1] = rotz[0][0];
    ogLibPntMatrix(rotz, 2, "zmatrix");

    ogLibMulMatrix(look, trans, roty);
    ogLibMulMatrix(look, look, rotx);
    ogLibMulMatrix(look, look, rotz);
    ogLibPntMatrix(look, 2, "lookat");

    /* OGLXXX MSINGLE not supported */
    if (GtstMmode == GL_MODELVIEW)
	ogLibMulMatrix(Comp, look, Comp);
    else if (GtstMmode == GL_MODELVIEW) {
	ogLibMulMatrix(View, look, View);
	ogLibMulMatrix(Comp, View, Prsp);
    } else if (GtstMmode == GL_PROJECTION) {
	ogLibMulMatrix(Prsp, look, Prsp);
	ogLibMulMatrix(Comp, View, Prsp);
    } else if (GtstMmode == GL_TEXTURE)
	ogLibMulMatrix(Mtex, look, Mtex);
    else
	ogEnvLog(OG_LINTERNALERROR, "gtst_lookat()\n");

    START_DL_OR_IM(1)
    /* OGLXXX lookat: replace UPx with vector */
	gluLookat(vx, vy, vz, px, py, pz, UPX(twist), UPY(twist), UPZ(twist));
    /* OGLXXX lookat: replace UPx with vector */
    ogEnvLog(1, "gluLookat(%f, %f, %f, %f, %f, %f, UPX(%d), UPY(%d), UPZ(%d));\n", vx, vy, vz, px, py, pz, twist);
    FINIS_DL_OR_IM(1)
#else
    ogEnvLog(OG_LALWAYS, "gtst_lookat() - unimplemented!!!\n");
#endif				/* FIX_XXX */
}

/*************************************************************
*  gtst_polarview()  - 
*************************************************************/
void 
gtst_polarview(float dist, short azim, short inc, short twist)
{
    float polar[4][4], trans[4][4], roty[4][4], rotx[4][4], rotz[4][4];

    ogLibUntMatrix(trans[0]);
    trans[3][2] = -dist;

    tx_frotmat(-azim / 10.0, 'z', roty);
    tx_frotmat(-inc / 10.0, 'x', rotx);
    tx_frotmat(-twist / 10.0, 'z', rotz);

    ogLibMulMatrix(polar[0], roty[0], rotx[0]);
    ogLibMulMatrix(polar[0], polar[0], rotz[0]);
    ogLibMulMatrix(polar[0], polar[0], trans[0]);
    ogLibPntMatrix(polar[0], 2, "polarview");

    if (GtstMmode == GL_MODELVIEW) {
	ogLibMulMatrix(View[0], polar[0], View[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_PROJECTION) {
	ogLibMulMatrix(Prsp[0], polar[0], Prsp[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else if (GtstMmode == GL_TEXTURE)
	ogLibMulMatrix(Mtex[0], polar[0], Mtex[0]);
    else
	ogEnvLog(OG_LINTERNALERROR, "gtst_polarview()\n");

    START_DL_OR_IM(1)
    /*
     * OGLXXX polarview not supported, use: glRotatef(angle, x, y, z);
     * glTranslatef(x, y, z); polarview(dist,azim,inc,twist) 
     */
	 /* DELETED */ ;
    /*
     * OGLXXX polarview not supported, use: glRotatef(angle, x, y, z);
     * glTranslatef(x, y, z); polarview(%f,%d,%d,%d) 
     */
    ogEnvLog(1, "/*DELETED*/;\n", dist, azim, inc, twist);
    FINIS_DL_OR_IM(1)
}

/*************************************************************
*  gtst_perspective()  - 
*************************************************************/
void 
gtst_perspective(short fovy, float aspect, float znear, float zfar)
{
    float persp[4][4];
    float ctt, Zdelta;

    Zdelta = zfar - znear;

    ctt = cos(M_PI * fovy / 3600.0) / sin(M_PI * fovy / 3600.0);

    ogLibUntMatrix(persp[0]);
    persp[0][0] = ctt / aspect;
    persp[1][1] = ctt;
    persp[2][2] = -(zfar + znear) / Zdelta;
    persp[2][3] = -1.0;
    persp[3][2] = -2.0 * znear * zfar / Zdelta;
    persp[3][3] = 0.0;
    ogLibPntMatrix(persp[0], 2, "perspective");

    /* OGLXXX MSINGLE not supported */
    if (GtstMmode == GL_MODELVIEW)
	ogLibCpyMatrix(Comp[0], persp[0]);
    else if (GtstMmode == GL_MODELVIEW || GtstMmode == GL_PROJECTION || GtstMmode == GL_TEXTURE) {
	ogLibCpyMatrix(Prsp[0], persp[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
    } else
	ogEnvLog(OG_LINTERNALERROR, "gtst_perspective()\n");

    START_DL_OR_IM(1)
	gluPerspective((GLdouble) (.1 * (fovy)), (GLdouble) aspect, (GLdouble) znear, (GLdouble) zfar);
    ogEnvLog(1, "gluPerspective((GLdouble)(.1*(%d)), (GLdouble)%f, (GLdouble)%f, (GLdouble)%f);\n", fovy, aspect, znear, zfar);
    FINIS_DL_OR_IM(1)
}

/*************************************************************
*  gtst_ortho2()  - 
*************************************************************/
void 
gtst_ortho2(float x1, float x2, float y1, float y2)
{
    float orto[4][4];

    ogLibUntMatrix(orto[0]);
    orto[0][0] = 2.0 / (x2 - x1);
    orto[1][1] = 2.0 / (y2 - y1);
    orto[2][2] = -1;
    orto[3][0] = -(x2 + x1) / (x2 - x1);
    orto[3][1] = -(y2 + y1) / (y2 - y1);
    ogLibPntMatrix(orto[0], 2, "ortho2");

    switch (GtstMmode) {
    case GL_MODELVIEW:
	ogLibMulMatrix(View[0], orto[0], View[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
	break;
    case GL_PROJECTION:
	ogLibMulMatrix(Prsp[0], orto[0], Prsp[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
	break;
    case GL_TEXTURE:
	ogLibMulMatrix(Mtex[0], orto[0], Mtex[0]);
	break;
    default:
	ogEnvLog(OG_LINTERNALERROR, "gtst_ortho()\n");
    }

    START_DL_OR_IM(1)
	gluOrtho2D(x1, x2, y1, y2);
    ogEnvLog(1, "gluOrtho2D(%f, %f, %f, %f);\n", x1, x2, y1, y2);
    FINIS_DL_OR_IM(1)
}

/*************************************************************
*  gtst_ortho()  - 
*************************************************************/
void 
gtst_ortho(float x1, float x2, float y1, float y2, float z1, float z2)
{
    float orto[4][4];

    ogLibUntMatrix(orto[0]);
    orto[0][0] = 2.0 / (x2 - x1);
    orto[1][1] = 2.0 / (y2 - y1);
    orto[2][2] = -2.0 / (z2 - z1);
    orto[3][0] = -(x2 + x1) / (x2 - x1);
    orto[3][1] = -(y2 + y1) / (y2 - y1);
    orto[3][2] = -(z2 + z1) / (z2 - z1);
    ogLibPntMatrix(orto[0], 2, "ortho");

    switch (GtstMmode) {
    case GL_MODELVIEW:
	ogLibMulMatrix(View[0], orto[0], View[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
	break;
    case GL_PROJECTION:
	ogLibMulMatrix(Prsp[0], orto[0], Prsp[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
	break;
    case GL_TEXTURE:
	ogLibMulMatrix(Mtex[0], orto[0], Mtex[0]);
	break;
    default:
	ogEnvLog(OG_LINTERNALERROR, "gtst_ortho()\n");
    }

    START_DL_OR_IM(1)
	glOrtho(x1, x2, y1, y2, z1, z2);
    ogEnvLog(1, "glOrtho(%f, %f, %f, %f, %f, %f);\n", x1, x2, y1, y2, z1, z2);
    FINIS_DL_OR_IM(1)
}

/*************************************************************
*  gtst_frustum()  - 
*************************************************************/
void 
gtst_frustum(float x1, float x2, float y1, float y2, float z1, float z2)
{
    float frust[4][4];

    ogLibUntMatrix(frust[0]);
    frust[0][0] = 2.0 * z1 / (x2 - x1);
    frust[1][1] = 2.0 * z1 / (y2 - y1);
    frust[2][0] = (x2 + x1) / (x2 - x1);
    frust[2][1] = (y2 + y1) / (y2 - y1);
    frust[2][2] = -(z2 + z1) / (z2 - z1);
    frust[2][3] = -1.0;
    frust[3][2] = -2.0 * z1 * z2 / (z2 - z1);
    frust[3][3] = 0.0;
    ogLibPntMatrix(frust[0], 2, "frustum");

    switch (GtstMmode) {
    case GL_MODELVIEW:
	ogLibMulMatrix(View[0], frust[0], View[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
	break;
    case GL_PROJECTION:
	ogLibMulMatrix(Prsp[0], frust[0], Prsp[0]);
	ogLibMulMatrix(Comp[0], View[0], Prsp[0]);
	break;
    case GL_TEXTURE:
	ogLibMulMatrix(Mtex[0], frust[0], Mtex[0]);
	break;
    default:
	ogEnvLog(OG_LINTERNALERROR, "gtst_frustum()\n");
    }

    START_DL_OR_IM(1)
	glFrustum(x1, x2, y1, y2, z1, z2);
    ogEnvLog(1, "glFrustum(%f, %f, %f, %f, %f, %f);\n", x1, x2, y1, y2, z1, z2);
    FINIS_DL_OR_IM(1)
}

/*************************************************************
*  gtst_pick()  - 
*************************************************************/
/*ARGSUSED*/
void 
gtst_pick(short *buffer, GLint numnames)
{
#ifdef FIX_XXX
    float fx, fy, wxmin, wymin;
    short vxmin, vxmax, vymin, vymax;
    GLint xorg, yorg, xcurs, ycurs;

    /*
     * OGLXXX getorigin not supported -- See Window Manager getorigin(&xorg,
     * &yorg) 
     */
     /* DELETED */ ;
    wxmin = xorg;
    wymin = yorg;

    /*
     * OGLXXX getvaluator not supported -- See Events getvaluator(CURSORX) 
     */
    xcurs = /* DELETED */ ;
    /*
     * OGLXXX getvaluator not supported -- See Events getvaluator(CURSORY) 
     */
    ycurs = /* DELETED */ ;

    fx = 10.0;
    fy = 10.0;

    /*
     * OGLXXX get GL_VIEWPORT: You can probably do better than this. 
     */
    {
	GLint tmp[4];
	glGetFloatv(GL_VIEWPORT, &tmp);
	&vxmin = tmp[0];
	&vxmax = tmp[0] + tmp[2];
	&vymin = tmp[1];
	&vymax = tmp[1] + tmp[3];
    };

    ogLibUntMatrix(Pick);
    Pick[0][0] = (vxmax - vxmin + 1) / fx;
    Pick[1][1] = (vymax - vymin + 1) / fy;
    Pick[3][0] = (-2 * xcurs + 2 * wxmin + vxmin + vxmax) / fx;
    Pick[3][1] = (-2 * ycurs + 2 * wymin + vymin + vymax) / fy;
    ogLibPntMatrix(Pick, 2, "picking");

    pickmode = GL_TRUE;
    /*
     * OGLXXX pick: Select buffer is type GLuint. Set gluPickMatrix params.
     * See man pages. Might want to push Projection matrix if you have
     * endpick pop it. 
     */
    glSelectBuffer(numnames, buffer);
    glRenderMode(GL_SELECT);
    glMatrixMode(GL_PROJECTION);
    gluPickMatrix(x, y, w, h, viewport);
    glMatrixMode(GL_MODELVIEW);
    /*
     * OGLXXX pick: Select buffer is type GLuint. Set gluPickMatrix params.
     * See man pages. Might want to push Projection matrix if you have
     * endpick pop it. 
     */
    ogEnvLog(1, "glSelectBuffer(%d, 0x%x);glRenderMode(GL_SELECT);glMatrixMode(GL_PROJECTION);gluPickMatrix(x, y, w, h, viewport);glMatrixMode(GL_MODELVIEW);\n", buffer, numnames);
#else
    ogEnvLog(OG_LALWAYS, "gtst_pick() - unimplemented!!!\n");
#endif
}

/*************************************************************
*  gtst_endpick()  - 
*************************************************************/
/*ARGSUSED*/
GLint 
gtst_endpick(short *buffer)
{
#ifdef FIX_XXX
    GLint a;

    ogLibUntMatrix(Pick);
    pickmode = GL_FALSE;
    /*
     * OGLXXX endpick: replace gluPerspective args or use glPopMatrix() to
     * restore. 
     */
    a = glRenderMode(GL_RENDER);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovy, aspect, znear, zfar);
    glMatrixMode(GL_MODELVIEW);;
    /*
     * OGLXXX endpick: replace gluPerspective args or use glPopMatrix() to
     * restore. 
     */
    ogEnvLog(1, "%d = glRenderMode(GL_RENDER); glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluPerspective( fovy, aspect, znear, zfar ); glMatrixMode(GL_MODELVIEW);;\n", a, buffer);
    return (a);
#else
    ogEnvLog(OG_LALWAYS, "gtst_endpick() - unimplemented!!!\n");
    return 0;
#endif
}


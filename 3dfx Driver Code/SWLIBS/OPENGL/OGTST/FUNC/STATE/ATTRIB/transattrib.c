#include <stdio.h>
#include "ogtst.h"

#define EPSILON 0.0001
#define NEQUAL(a, b) (!(fabs((a) - (b)) < EPSILON))

#define COMPARE3(class, index, attr, a, b) do {                         \
    if (NEQUAL((a)[0], (b)[0]) || NEQUAL((a)[1], (b)[1]) ||             \
        NEQUAL((a)[2], (b)[2]))                                         \
        ogEnvLog(OG_LFAIL,                                              \
                 #class " %d " #attr " (%g %g %g) != (%g %g %g)\n",     \
                 index, a[0], a[1], a[2],                               \
                 b[0], b[1], b[2]);                                     \
} while (0)

#define COMPARE4(class, index, attr, a, b) do {                                 \
    if (NEQUAL((a)[0], (b)[0]) || NEQUAL((a)[1], (b)[1]) ||                     \
        NEQUAL((a)[2], (b)[2]) || NEQUAL((a)[3], (b)[3]))                       \
        ogEnvLog(OG_LFAIL,                                                      \
                 #class " %d " #attr " (%g %g %g %g) != (%g %g %g %g)\n",       \
                 index, a[0], a[1], a[2], a[3],                                 \
                 b[0], b[1], b[2], b[3]);                                       \
} while (0)

#define COMPARE4d(class, index, attr, a, b) do {                                 \
    if (NEQUAL((a)[0], (b)[0]) || NEQUAL((a)[1], (b)[1]) ||                     \
        NEQUAL((a)[2], (b)[2]) || NEQUAL((a)[3], (b)[3]))                       \
        ogEnvLog(OG_LFAIL,                                                      \
                 #class " %d " #attr " (%lg %lg %lg %lg) != (%lg %lg %lg %lg)\n",\
                 index, a[0], a[1], a[2], a[3],                                 \
                 b[0], b[1], b[2], b[3]);                                       \
} while (0)

#define TEST(t) \
    ogEnvLog(2, #t"\n"); \
    t

#define HAVE_EXT(ext) (strstr((const char *)ex, ext) != 0)

/*ARGSUSED*/
TESTMOD(transattrib)  {
    const GLubyte *ex = glGetString(GL_EXTENSIONS);
    GLfloat vec1[4], vec2[4], ret[4], dir[3];
    GLdouble vec1d[4], retd[4];
    GLfloat matrix[16];
    float vlen;
    int i, max;

    for (i = 0; i < 3; i++)
        vec1[i] = ogLibFloatRand(-10, 10);
    vec1[3] = ogLibBitRand(1);
    do {
        for (i = 0; i < 3; i++)
            dir[i] = ogLibFloatRand(-5, 5);
        vlen = sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
    } while (vlen < EPSILON);
    /* Normalize the direction */
    for (i = 0; i < 3; i++)
        dir[i] /= vlen;
    for (i = 0; i < 16; i++)
        matrix[i] = ogLibFloatRand(-20, 20);
    
    glGetIntegerv(GL_MAX_LIGHTS, &max);
    for (i = 0; i < max; i++) {
	ogEnvLog(1, "Test light %d\n", i);
	glPushMatrix();
	TEST(glLightfv(GL_LIGHT0+i, GL_POSITION, vec1));
	TEST(glLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, dir));
	TEST(glPushAttrib(GL_LIGHTING_BIT));
	glGetLightfv(GL_LIGHT0+i, GL_POSITION, ret);
	COMPARE4(LIGHT, i, POSITIION, ret, vec1);
	glGetLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, ret);
	COMPARE3(LIGHT, i, SPOT_DIRECTION, ret, dir);
	TEST(glLoadMatrixf(matrix));
	glLightfv(GL_LIGHT0+i, GL_POSITION, vec1);
	glLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, dir);
	TEST(glPopAttrib());
	glGetLightfv(GL_LIGHT0+i, GL_POSITION, ret);
	COMPARE4(LIGHT, i, POSITIION, ret, vec1);
	glGetLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, ret);
	COMPARE3(LIGHT, i, SPOT_DIRECTION, ret, dir);
	glPopMatrix();
    }

    for (i = 0; i < 4; i++)
        vec1d[i] = ogLibFloatRand(-10, 10);
    glGetIntegerv(GL_MAX_CLIP_PLANES, &max);
    for (i = 0; i < max; i++) {
	ogEnvLog(1, "Test clip plane %d\n", i);
	glPushMatrix();
	TEST(glClipPlane(GL_CLIP_PLANE0+i, vec1d));
	TEST(glPushAttrib(GL_TRANSFORM_BIT));
	glGetClipPlane(GL_CLIP_PLANE0+i, retd);
	COMPARE4d(CLIP_PLANE, i, "", retd, vec1d);
	TEST(glLoadMatrixf(matrix));
	glClipPlane(GL_CLIP_PLANE0+i, vec1d);
	TEST(glPopAttrib());
	glGetClipPlane(GL_CLIP_PLANE0+i, retd);
	COMPARE4d(CLIP_PLANE, i, "", retd, vec1d);
	glPopMatrix();
    }
    ogEnvLog(1, "Test texgen object and eye\n");
    for (i = 0; i < 4; i++) {
        vec1[i] = ogLibFloatRand(-10, 10);
        vec2[i] = ogLibFloatRand(-10, 10);
    }
    for (i = GL_S; i <= GL_Q; i++) {
	glPushMatrix();
	TEST(glTexGenfv(i, GL_OBJECT_PLANE, vec1));
	TEST(glTexGenfv(i, GL_EYE_PLANE, vec2));
	TEST(glPushAttrib(GL_TEXTURE_BIT));
	glGetTexGenfv(i, GL_OBJECT_PLANE, ret);
	COMPARE4(TexEnv(OBJECT_PLANE), i - GL_S, "",ret, vec1);
	glGetTexGenfv(i, GL_EYE_PLANE, ret);
	COMPARE4(TexEnv(EYE_PLANE), i - GL_S, "",ret, vec2);
	TEST(glLoadMatrixf(matrix));
	glTexGenfv(i, GL_OBJECT_PLANE, vec1);
	TEST(glTexGenfv(i, GL_EYE_PLANE, vec2));
	TEST(glPopAttrib());
	glGetTexGenfv(i, GL_OBJECT_PLANE, ret);
	COMPARE4(TexEnv(OBJECT_PLANE), i - GL_S, "",ret, vec1);
	glGetTexGenfv(i, GL_EYE_PLANE, ret);
	COMPARE4(TexEnv(EYE_PLANE), i - GL_S, "",ret, vec2);
	glPopMatrix();
    }

}

CLEANUP(transattrib) {
    int i, max;
    double zero[] = {0, 0, 0, 0};
    double s[] = {1, 0, 0, 0}, t[] = {0, 1, 0, 0};
    float pos[] = {0, 0, 1, 0}, dir[] = {0, 0, -1};
    const GLubyte *ex = glGetString(GL_EXTENSIONS);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glGetIntegerv(GL_MAX_LIGHTS, &max);
    for (i = 0; i < max; i++) {
	glLightfv(GL_LIGHT0+i, GL_POSITION, pos);
	glLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, dir);
    }

    glGetIntegerv(GL_MAX_CLIP_PLANES, &max);
    for (i = 0; i < max; i++) {
	glClipPlane(GL_CLIP_PLANE0+i, zero);
    }

    glTexGendv(GL_S, GL_OBJECT_PLANE, s);
    glTexGendv(GL_S, GL_EYE_PLANE, s);
    glTexGendv(GL_T, GL_OBJECT_PLANE, t);
    glTexGendv(GL_T, GL_EYE_PLANE, t);
    glTexGendv(GL_R, GL_OBJECT_PLANE, zero);
    glTexGendv(GL_R, GL_EYE_PLANE, zero);
    glTexGendv(GL_Q, GL_OBJECT_PLANE, zero);
    glTexGendv(GL_Q, GL_EYE_PLANE, zero);
}

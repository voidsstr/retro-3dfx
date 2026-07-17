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

/* copytex.c - $Revision: 2$ */

/*
 * This program tests the copy texture 1.1.
 * It does glCopyTexSumImage1D, glCopyTexImage1D, 
 * glCopyTexSubImage2D, and glCopyTexImage2D for the
 * same base texture and internal format.  Then it tests
 * various internalformats passed to glCopyTexImage2D.
 */

#include "ogtst.h"
#include "GL/glu.h"

#ifdef GL_VERSION_1_1

static GLenum target = GL_TEXTURE_2D;
static GLenum internalformat = GL_RGB;

#define TEXSIZE 128
#define NUM_TEXTURES 3

static GLint xoffset = TEXSIZE>>2, yoffset = TEXSIZE>>2;


typedef struct {
    GLint x;
    GLint y;
    GLint z;
    GLfloat width;
    GLfloat height;
    GLint border;
    GLdouble modelMatrix[16];
    GLdouble projMatrix[16];
    GLint viewport[4];
} boxStruct;
static boxStruct copyBox;

static GLubyte texData1[4][3] = {
    {0xFF,0x00,0x00},
    {0x00,0xFF,0x00}, 
    {0x00,0x00,0xFF},
    {0xFF,0xFF,0x00}, 
};

static GLubyte texData2[4][3] = {
    {0x00,0x00,0x00},
    {0xFF,0xFF,0xFF}, 
    {0x00,0x00,0x00},
    {0xFF,0xFF,0xFF}, 
};

static GLubyte texBuf[NUM_TEXTURES][TEXSIZE][TEXSIZE][3];

static float repeat[] = {GL_REPEAT};
static float nr[] = {GL_NEAREST};


static float c[4][3] = {
    { -1.0, 1.0, 1.0 },
    { 1.0, 1.0, 1.0 },
    { 1.0, -1.0, 1.0 },
    { -1.0, -1.0, 1.0 }
};
static float n[3] = { 0.0, 0.0, 1.0 };
static float t[4][2] = {
    { 0.0, 1.0 }, { 1.0, 1.0 }, { 1.0, 0.0 }, { 0.0,  0.0 }
};


static void SetCopyBox(void)
{
    GLdouble x, y, z;

    glGetDoublev(GL_PROJECTION_MATRIX, copyBox.projMatrix);
    glGetDoublev(GL_MODELVIEW_MATRIX, copyBox.modelMatrix);
    glGetIntegerv(GL_VIEWPORT, copyBox.viewport);
    gluProject(0.0, -0.98, 1.0, 
		copyBox.modelMatrix, copyBox.projMatrix, copyBox.viewport,
		&x, &y, &z);
    copyBox.border = 1;
    copyBox.width = copyBox.height = (TEXSIZE>>1) + 2*copyBox.border;
    copyBox.x = x;
    copyBox.y = y;
    copyBox.z = z;
}


static void SelectTexture(GLint i)
{
    if (target == GL_TEXTURE_2D) {
	glTexImage2D(GL_TEXTURE_2D, 0, 3, TEXSIZE, TEXSIZE, 0,
			GL_RGB, GL_UNSIGNED_BYTE, texBuf[i]); }
    else {
	glTexImage1D(GL_TEXTURE_1D, 0, 3, TEXSIZE, 0,
			GL_RGB, GL_UNSIGNED_BYTE, texBuf[i]);
    }
}

static void DrawSquare(void)
{
    glBegin(GL_POLYGON);
	glNormal3fv(n); glTexCoord2fv(t[0]); glVertex3fv(c[0]);
	glNormal3fv(n); glTexCoord2fv(t[1]); glVertex3fv(c[1]);
	glNormal3fv(n); glTexCoord2fv(t[2]); glVertex3fv(c[2]);
	glNormal3fv(n); glTexCoord2fv(t[3]); glVertex3fv(c[3]);
    glEnd();
    glFinish(); /* temp workaround for RE3 */
}

static void InitTex(void)
{
    int i,j,k;
    int smallSquare = TEXSIZE>>3;
    int mediumSquare = TEXSIZE>>2;

    k=0;
    for (i=0; i < TEXSIZE; i++) {
	if (i%smallSquare == 0) k = (k+1)%4;
	for (j=0; j < TEXSIZE; j++) {
	    if (j%smallSquare == 0) k = (k+1)%4;
	    memcpy(texBuf[0][i][j], &texData1[k], 3);
	}
    }
    k=0;
    for (i=0; i < TEXSIZE; i++) {
	if (i%smallSquare == 0) k = (k+1)%4;
	for (j=0; j < TEXSIZE; j++) {
	    if (j%smallSquare == 0) k = (k+1)%4;
	    memcpy(texBuf[1][i][j], &texData2[k], 3);
	}
    }
    k=0;
    for (i=0; i < TEXSIZE; i++) {
	if (i%mediumSquare == 0) k = (k+1)%4;
	for (j=0; j < TEXSIZE; j++) {
	    if (j%mediumSquare == 0) k = (k+1)%4;
	    memcpy(texBuf[2][i][j], &texData1[k], 3);
	}
    }
}

static void Init(void)
{
    InitTex();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glColor3f(1.0, 1.0, 1.0);

    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, nr);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, nr);

    glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, repeat);
    glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, repeat);
    glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, nr);
    glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, nr);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0, 1.0, 0.01, 1000);
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(-3.1, 3.3, -5.8);
}


static void Draw(void)
{
    GLint obj; /* used by DL_OR_IM macros */

    glTranslatef(2.1, 0.0, 0.0);
    SetCopyBox();
    glTranslatef(-2.1, 0.0, 0.0);

    START_DL_OR_IM(1);

	glPushMatrix();

	glClear(GL_COLOR_BUFFER_BIT);

	target = GL_TEXTURE_1D;
	glEnable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);

	SelectTexture(0);
	DrawSquare();

	target = GL_TEXTURE_2D;
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);


	SelectTexture(1);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();

	SelectTexture(0);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();

	SelectTexture(2);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();

	glFlush();

	target = GL_TEXTURE_1D;
	glEnable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);

	glTranslatef(-8.4, -2.1, 0.0);

	SelectTexture(0);
	glCopyTexSubImage1D(GL_TEXTURE_1D, 0, xoffset,
			    copyBox.x, copyBox.y, copyBox.width);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();
	glCopyTexImage1D(GL_TEXTURE_1D, 0, internalformat,
			copyBox.x, copyBox.y, copyBox.width, copyBox.border);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();


	target = GL_TEXTURE_2D;
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);


	SelectTexture(0);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset,
			copyBox.x, copyBox.y, copyBox.width, copyBox.height);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();
	glCopyTexImage2D(GL_TEXTURE_2D, 0, internalformat, 
			    copyBox.x, copyBox.y, 
			    copyBox.width, copyBox.height, copyBox.border);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();


	glTranslatef(-8.4, -2.1, 0.0);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 
			    copyBox.x, copyBox.y, 
			    copyBox.width, copyBox.height, copyBox.border);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, 
			    copyBox.x, copyBox.y, 
			    copyBox.width, copyBox.height, copyBox.border);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, 
			    copyBox.x, copyBox.y, 
			    copyBox.width, copyBox.height, copyBox.border);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
			    copyBox.x, copyBox.y, 
			    copyBox.width, copyBox.height, copyBox.border);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();


	glTranslatef(-8.4, -2.1, 0.0);

	SelectTexture(2);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_DST_ALPHA);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 
			    copyBox.x, copyBox.y, 
			    copyBox.width, copyBox.height, copyBox.border);
	DrawSquare();
	glDisable(GL_BLEND);

	SelectTexture(2);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();
	glEnable(GL_BLEND);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA12, 
			    copyBox.x, copyBox.y, 
			    copyBox.width, copyBox.height, copyBox.border);
	DrawSquare();
	glDisable(GL_BLEND);

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY, 
			    copyBox.x, copyBox.y, 
			    copyBox.width, copyBox.height, copyBox.border);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY8,
			    copyBox.x, copyBox.y, 
			    copyBox.width, copyBox.height, copyBox.border);
	glTranslatef(2.1, 0.0, 0.0);
	DrawSquare();

	glFlush();
	glPopMatrix();

    FINIS_DL_OR_IM(1);

}


static void cleanup(void)
{
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0,0,0,0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER,
		    GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		    GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    ogLibSetDefaultTextures();
    glTexImage1D(GL_TEXTURE_1D, 0, 1, 0, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);


    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
}
#endif /* GL_VERSION_1_1 */


TESTMOD(copytex)
{
#ifdef GL_VERSION_1_1
    if (IS_ONEONE()) {

       Init();

       while (pass--) {
	   Draw();
       }

    }
#endif
}

CLEANUP(copytex)
{
#ifdef GL_VERSION_1_1
    cleanup();
#endif
}



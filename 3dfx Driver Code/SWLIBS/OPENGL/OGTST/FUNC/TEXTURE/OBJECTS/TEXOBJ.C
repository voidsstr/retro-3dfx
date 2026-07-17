#include "ogtst.h"
#include "GL/glu.h"

#ifdef GL_VERSION_1_1 

static GLenum target = GL_TEXTURE_2D;

#define TEXSIZE 64
#define NUM_TEXTURES 3

static GLuint texNames1D[NUM_TEXTURES];
static GLuint texNames2D[NUM_TEXTURES];
static GLfloat texPriorities[3] = {0.1, 0.2, 0.3};

static GLubyte texData[4][3] = {
    {0xFF,0x00,0x00},
    {0x00,0xFF,0x00}, 
    {0x00,0x00,0xFF},
    {0xFF,0xFF,0x00}, 
};

static GLubyte texBuf[NUM_TEXTURES][TEXSIZE][TEXSIZE][3];

static float decal[] = {GL_DECAL};
static float repeat[] = {GL_REPEAT};
static float clamp[] = {GL_CLAMP};
static float nr[] = {GL_NEAREST};


static float c[4][3] = {
    { -1.0, 1.0, 1.0 },
    { 1.0, 1.0, 1.0 },
    { 1.0, -1.0, 1.0 },
    { -1.0, -1.0, 1.0 }
};
static float n[3] = { 0.0, 0.0, 1.0 };
static float t[4][2] = {
    { -0.1,  1.1 },
    { 1.1, 1.1 },
    { 1.1, -0.1 },
    { -0.1,  -0.1 }
};


static void SelectTexture(GLint i)
{
    GLuint *texNames;

    texNames = (target == GL_TEXTURE_1D) ? texNames1D : texNames2D;
    if (i<NUM_TEXTURES)
	glBindTexture(target, texNames[i]);
    else
	glBindTexture(target, 0);
    return;
}



static void DrawSquare(void)
{
    glBegin(GL_POLYGON);
	glNormal3fv(n); glTexCoord2fv(t[0]); glVertex3fv(c[0]);
	glNormal3fv(n); glTexCoord2fv(t[1]); glVertex3fv(c[1]);
	glNormal3fv(n); glTexCoord2fv(t[2]); glVertex3fv(c[2]);
	glNormal3fv(n); glTexCoord2fv(t[3]); glVertex3fv(c[3]);
    glEnd();
}

static void DrawSquares(void)
{
    GLint i;

    glTranslatef(-2.0, 0.0, 0.0);
    for (i = 0; i < 3; i++) {
	SelectTexture(i);
	DrawSquare();
	glTranslatef(2.0, 0.0, 0.0);
    }
}


static void DoPushPopTest(void)
{
    SelectTexture(1);
    glTranslatef(-2.0, 0.0, 0.0);
    DrawSquare();

    glPushAttrib(GL_TEXTURE_BIT);
    SelectTexture(2);
    glTexParameterfv(target, GL_TEXTURE_WRAP_S, clamp);
    glTexParameterfv(target, GL_TEXTURE_WRAP_T, clamp);
    glTranslatef(2.0, 0.0, 0.0);
    DrawSquare();

    glPopAttrib();
    glTranslatef(2.0, 0.0, 0.0);
    DrawSquare();

    glTexParameterfv(target, GL_TEXTURE_WRAP_S, clamp);
    glTexParameterfv(target, GL_TEXTURE_WRAP_T, clamp);
    glTranslatef(-4.0, -2.0, 0.0);
    DrawSquare();

    SelectTexture(0);
    glTranslatef(2.0, 0.0, 0.0);
    DrawSquare();

    SelectTexture(2);
    glTranslatef(2.0, 0.0, 0.0);
    DrawSquare();

}

static void InitTex(void)
{
    int i,j,k;

    k=0;
    for (i=0; i < TEXSIZE; i++) {
	if (i%8 == 0) k = (k+1)%4;
	for (j=0; j < TEXSIZE; j++) {
	    if (j%8 == 0) k = (k+1)%4;
	    memcpy(texBuf[0][i][j], &texData[k], 3);
	}
    }
    k=0;
    for (i=0; i < TEXSIZE; i++) {
	if (i%16 == 0) k = (k+1)%4;
	for (j=0; j < TEXSIZE; j++) {
	    if (j%8 == 0) k = (k+1)%4;
	    memcpy(texBuf[1][i][j], &texData[k], 3);
	}
    }
    k=0;
    for (i=0; i < TEXSIZE; i++) {
	if (i%16 == 0) k = (k+1)%4;
	for (j=0; j < TEXSIZE; j++) {
	    if (j%16 == 0) k = (k+1)%4;
	    memcpy(texBuf[2][i][j], &texData[k], 3);
	}
    }
}


static void Init(void)
{
    int i;

    InitTex();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(NUM_TEXTURES, texNames2D);
    glGenTextures(NUM_TEXTURES, texNames1D);

    for (i=0; i < NUM_TEXTURES; i++) {
	glBindTexture(GL_TEXTURE_2D, texNames2D[i]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, TEXSIZE, TEXSIZE, 0,
			GL_RGB, GL_UNSIGNED_BYTE, texBuf[i]);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, nr);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, nr);

	glBindTexture(GL_TEXTURE_1D, texNames1D[i]);
	glTexImage1D(GL_TEXTURE_1D, 0, 3, TEXSIZE, 0,
			GL_RGB, GL_UNSIGNED_BYTE, texBuf[i]);
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, repeat);
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, repeat);
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, nr);
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, nr);
    }

    glPrioritizeTextures(NUM_TEXTURES, texNames2D, texPriorities);
    glPrioritizeTextures(NUM_TEXTURES, texNames1D, texPriorities);

    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, decal);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(145.0, 1.0, 0.01, 1000);
    glMatrixMode(GL_MODELVIEW);
}

static void Draw(void)
{
    GLint obj; /* used by DL_OR_IM macros */

    START_DL_OR_IM(1);

    glClear(GL_COLOR_BUFFER_BIT);

    glTranslatef(0.0, 2.8, -2.5);
    glPushMatrix();
    target = GL_TEXTURE_1D;
    glEnable(GL_TEXTURE_1D);

    DrawSquares();
    glPopMatrix();

    glTranslatef(0.0, -2.0, 0.0);
    glPushMatrix();
    target = GL_TEXTURE_2D;
    glEnable(GL_TEXTURE_2D);

    DrawSquares();
    glPopMatrix();

    glTranslatef(0.0, -2.0, 0.0);

    DoPushPopTest();

    glFlush();

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

    glDeleteTextures(NUM_TEXTURES, texNames1D);
    glDeleteTextures(NUM_TEXTURES, texNames2D);

    ogLibSetDefaultTextures();
}
#endif


TESTMOD(texobj)
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

CLEANUP(texobj)
{
#ifdef GL_VERSION_1_1 
    cleanup();
#endif
}



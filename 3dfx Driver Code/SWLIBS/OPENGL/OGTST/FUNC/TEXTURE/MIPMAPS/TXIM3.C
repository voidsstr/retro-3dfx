/*
 * txim3.c - test immediate mode 3 components texture
 */

#include "ogtst.h"       /* include test environment */


#define COMPON	3

#define TEXSIZE_0	256
#define TEXSIZE_1	( ((TEXSIZE_0 >> 1) > 0) ? (TEXSIZE_0 >> 1) : 1)
#define TEXSIZE_2	( ((TEXSIZE_1 >> 1) > 0) ? (TEXSIZE_1 >> 1) : 1)
#define TEXSIZE_3	( ((TEXSIZE_2 >> 1) > 0) ? (TEXSIZE_2 >> 1) : 1)
#define TEXSIZE_4	( ((TEXSIZE_3 >> 1) > 0) ? (TEXSIZE_3 >> 1) : 1)
#define TEXSIZE_5	( ((TEXSIZE_4 >> 1) > 0) ? (TEXSIZE_4 >> 1) : 1)
#define TEXSIZE_6	( ((TEXSIZE_5 >> 1) > 0) ? (TEXSIZE_5 >> 1) : 1)
#define TEXSIZE_7	( ((TEXSIZE_6 >> 1) > 0) ? (TEXSIZE_6 >> 1) : 1)
#define TEXSIZE_8	( ((TEXSIZE_7 >> 1) > 0) ? (TEXSIZE_7 >> 1) : 1)
#define TEXSIZE_9	( ((TEXSIZE_8 >> 1) > 0) ? (TEXSIZE_8 >> 1) : 1)
#define TEXSIZE_10	( ((TEXSIZE_9 >> 1) > 0) ? (TEXSIZE_9 >> 1) : 1)
#define TEXSIZE_11	( ((TEXSIZE_10 >> 1) > 0) ? (TEXSIZE_10 >> 1) : 1)
#define TEXSIZE_12	( ((TEXSIZE_11 >> 1) > 0) ? (TEXSIZE_11 >> 1) : 1)

#define TEXSIZE_0_4	(TEXSIZE_0 >> 2)
#define ONE 0xffff
#define NUM_COLORS	6

static GLfloat t0[TEXSIZE_0*TEXSIZE_0*COMPON], t1[TEXSIZE_1*TEXSIZE_1*COMPON];
static GLfloat t2[TEXSIZE_2*TEXSIZE_2*COMPON], t3[TEXSIZE_3*TEXSIZE_3*COMPON];
static GLfloat t4[TEXSIZE_4*TEXSIZE_4*COMPON], t5[TEXSIZE_5*TEXSIZE_5*COMPON];
static GLfloat t6[TEXSIZE_6*TEXSIZE_6*COMPON], t7[TEXSIZE_7*TEXSIZE_7*COMPON];
static GLfloat t8[TEXSIZE_8*TEXSIZE_8*COMPON], t9[TEXSIZE_9*TEXSIZE_9*COMPON];
static GLfloat t10[TEXSIZE_10*TEXSIZE_10*COMPON];
static GLfloat t11[TEXSIZE_11*TEXSIZE_11*COMPON];
static GLfloat t12[TEXSIZE_12*TEXSIZE_12*COMPON];
static GLfloat *tplain[13] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12};

static GLushort tpattern[TEXSIZE_0*TEXSIZE_0*COMPON];


static void drawLevels(GLboolean transRight);

/*ARGSUSED*/
TESTMOD(txim3)
{
    GLint width, height, lod, size, i, j, s, t;
    GLfloat *texel;
    GLfloat color[NUM_COLORS][COMPON] = {{1.0, 0, 0}, {0, 1.0, 0}, 
		{0, 0, 1.0}, {1.0, 1.0, 0}, {0, 1.0, 1.0}, {1.0, 0, 1.0}};

	/* init texture */
    for ( lod = 0, size = TEXSIZE_0; size > 0; size >>= 1, lod ++ ){
	for (i = 0, texel = tplain[lod]; i < size * size; i ++, texel +=COMPON){
	    for ( j = 0; j < COMPON; j ++ )
		texel[j] = color[lod % NUM_COLORS][j];
	}
    }

    for (i = 0; i < TEXSIZE_0; i ++){
      for(j = 0; j < TEXSIZE_0; j ++){
	s = j / TEXSIZE_0_4;
	t = i / TEXSIZE_0_4;
	switch((s + t) % 4){
	  case 0:
            tpattern[i*TEXSIZE_0*COMPON + j*COMPON] = ONE;
            tpattern[i*TEXSIZE_0*COMPON + j*COMPON + 1] = ONE;
            tpattern[i*TEXSIZE_0*COMPON + j*COMPON + 2] = 0;
	    break;
	  case 1:
            tpattern[i*TEXSIZE_0*COMPON + j*COMPON] = ONE;
            tpattern[i*TEXSIZE_0*COMPON + j*COMPON + 1] = 0;
            tpattern[i*TEXSIZE_0*COMPON + j*COMPON + 2] = 0;
	    break;
	  case 2:
            tpattern[i*TEXSIZE_0*COMPON + j*COMPON] = 0;
            tpattern[i*TEXSIZE_0*COMPON + j*COMPON + 1] = ONE;
            tpattern[i*TEXSIZE_0*COMPON + j*COMPON + 2] = 0;
	    break;
	  case 3:
	    tpattern[i*TEXSIZE_0*COMPON + j*COMPON] = 0;
	    tpattern[i*TEXSIZE_0*COMPON + j*COMPON + 1] = 0;
	    tpattern[i*TEXSIZE_0*COMPON + j*COMPON + 2] = ONE;
	    break;
        }
      }
    }

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    width = ogEnvQuery(OG_XWSIZE);
    height = ogEnvQuery(OG_YWSIZE);
    glOrtho(0.0, width, 0.0, height, 0.01, 1000.0);

    glClearColor(0.25, 0.25, 0.25, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
						GL_LINEAR_MIPMAP_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);
    glColor3f(1.0, 1.0, 1.0);

    for ( lod = 0, size = TEXSIZE_0; size > 0; size >>= 1, lod ++ )
	glTexImage2D(GL_TEXTURE_2D, lod, 3, size, size, 0, GL_RGB, 
						GL_FLOAT, tplain[lod]);
    drawLevels(GL_TRUE);

    glLoadIdentity();
    glTranslatef(width - TEXSIZE_0 - 20.0, height - TEXSIZE_0 - 20.0, -10.0);

    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TEXSIZE_0, TEXSIZE_0, 
					GL_RGB, GL_UNSIGNED_SHORT, tpattern);
    drawLevels(GL_FALSE);

}

static void drawLevels(GLboolean transRight)
{
    GLint size;

    for (size = TEXSIZE_0; size > 0;){

	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(10.0, 10.0, -10.0); 

	glTexCoord2f(1.0, 0.0);
	glVertex3f(10.0 + size, 10.0, -10.0); 

	glTexCoord2f(1.0, 1.0);
	glVertex3f(10.0 + size, 10.0 + size, -10.0);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(10.0, 10.0 + size, -10.0); 
	glEnd();

	size >>= 1;

	if (transRight)
	    glTranslatef((size << 1) + 5.0, 0.0, 0.0);
	else
	    glTranslatef( -(size + 5.0), size, 0.0);
    }
    glFinish(); /* temp workaround for RE3 */
}

CLEANUP(txim3)
{
    glDisable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                                GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    ogLibSetDefaultTextures();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0, 0.0, 0.0, 0.0);
}


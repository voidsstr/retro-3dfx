/*
 * txdlim4_3.c - test combination of the display list and immediate mode 4 
 * components texture
 */

#include "ogtst.h"       /* include test environment */


#define COMPON	4

#define TEXSIZE_0_X	64
#define TEXSIZE_1_X	( ((TEXSIZE_0_X >> 1) > 0) ? (TEXSIZE_0_X >> 1) : 1)
#define TEXSIZE_2_X	( ((TEXSIZE_1_X >> 1) > 0) ? (TEXSIZE_1_X >> 1) : 1)
#define TEXSIZE_3_X	( ((TEXSIZE_2_X >> 1) > 0) ? (TEXSIZE_2_X >> 1) : 1)
#define TEXSIZE_4_X	( ((TEXSIZE_3_X >> 1) > 0) ? (TEXSIZE_3_X >> 1) : 1)
#define TEXSIZE_5_X	( ((TEXSIZE_4_X >> 1) > 0) ? (TEXSIZE_4_X >> 1) : 1)
#define TEXSIZE_6_X	( ((TEXSIZE_5_X >> 1) > 0) ? (TEXSIZE_5_X >> 1) : 1)
#define TEXSIZE_7_X	( ((TEXSIZE_6_X >> 1) > 0) ? (TEXSIZE_6_X >> 1) : 1)
#define TEXSIZE_8_X	( ((TEXSIZE_7_X >> 1) > 0) ? (TEXSIZE_7_X >> 1) : 1)

#define TEXSIZE_0_Y	128
#define TEXSIZE_1_Y	( ((TEXSIZE_0_Y >> 1) > 0) ? (TEXSIZE_0_Y >> 1) : 1)
#define TEXSIZE_2_Y	( ((TEXSIZE_1_Y >> 1) > 0) ? (TEXSIZE_1_Y >> 1) : 1)
#define TEXSIZE_3_Y	( ((TEXSIZE_2_Y >> 1) > 0) ? (TEXSIZE_2_Y >> 1) : 1)
#define TEXSIZE_4_Y	( ((TEXSIZE_3_Y >> 1) > 0) ? (TEXSIZE_3_Y >> 1) : 1)
#define TEXSIZE_5_Y	( ((TEXSIZE_4_Y >> 1) > 0) ? (TEXSIZE_4_Y >> 1) : 1)
#define TEXSIZE_6_Y	( ((TEXSIZE_5_Y >> 1) > 0) ? (TEXSIZE_5_Y >> 1) : 1)
#define TEXSIZE_7_Y	( ((TEXSIZE_6_Y >> 1) > 0) ? (TEXSIZE_6_Y >> 1) : 1)
#define TEXSIZE_8_Y	( ((TEXSIZE_7_Y >> 1) > 0) ? (TEXSIZE_7_Y >> 1) : 1)

#define MAJER_SIZE      ((TEXSIZE_0_X > TEXSIZE_0_Y)?TEXSIZE_0_X : TEXSIZE_0_Y)

static GLuint t0[TEXSIZE_0_X * TEXSIZE_0_Y * COMPON];
static GLuint t1[TEXSIZE_1_X * TEXSIZE_1_Y * COMPON];
static GLuint t2[TEXSIZE_2_X * TEXSIZE_2_Y * COMPON];
static GLuint t3[TEXSIZE_3_X * TEXSIZE_3_Y * COMPON];
static GLuint t4[TEXSIZE_4_X * TEXSIZE_4_Y * COMPON];
static GLuint t5[TEXSIZE_5_X * TEXSIZE_5_Y * COMPON];
static GLuint t6[TEXSIZE_6_X * TEXSIZE_6_Y * COMPON];
static GLuint t7[TEXSIZE_7_X * TEXSIZE_7_Y * COMPON];
static GLuint t8[TEXSIZE_8_X * TEXSIZE_8_Y * COMPON];

static GLuint *tpattern[13] = {t0, t1, t2, t3, t4, t5, t6, t7, t8};

#define NUM_COLORS	6
#define ONE 0xffffffff
#define A_1 0x22222222
#define A_2 0x77777777
#define A_3 0xaaaaaaaa
#define A_4 0xcccccccc
#define A_5 0xeeeeeeee
#define A_6 0xffffffff

static GLuint color[NUM_COLORS][COMPON] = {{ONE, 0, 0, A_1}, {0, ONE, 0, A_2},
  {0, 0, ONE, A_3}, {ONE, ONE, 0, A_4}, {0, ONE, ONE, A_5}, {ONE, 0, ONE, A_6}};

#define ONE_DLIST	GL_TRUE
#define MANY_DLIST	GL_FALSE

static void drawLevels(void);
static void defineLevels(GLint , GLenum, GLboolean, GLint);

/*ARGSUSED*/
TESTMOD(txdlim4_3)
{
    GLint width, height, lod, majer, size_x, size_y, i, j;
    GLuint *t;
    GLfloat xOff;

	/* define dlist texture images */
    majer = (TEXSIZE_0_X > TEXSIZE_0_Y) ? TEXSIZE_0_X : TEXSIZE_0_Y;
    size_x = TEXSIZE_0_X;
    size_y = TEXSIZE_0_Y;
    for ( lod = 0; majer > 0; majer >>= 1, lod ++ ){
        	/* init texture */
	for (i = 0, t = tpattern[lod]; i < size_x * size_y; i ++, t += COMPON){
	    if ((i % size_x) < (size_x >> 1)){
		if (i < (size_x * (size_y >> 1))){
		    for ( j = 0; j < COMPON; j ++ )
			t[j] = color[lod % NUM_COLORS][j];
		}else{ 
		    for ( j = 0; j < COMPON; j ++ )
			t[j] = color[(lod + 1) % NUM_COLORS][j];
		}
	    }else{
		if (i < (size_x * (size_y >> 1))){
		    for ( j = 0; j < COMPON; j ++ )
			t[j] = color[(lod + 2) % NUM_COLORS][j];
		}else{ 
		    for ( j = 0; j < COMPON; j ++ )
			t[j] = color[(lod + 3) % NUM_COLORS][j];
		}
	    }
	}
	size_x = size_x >> 1;
	if (size_x == 0) size_x = 1;
	size_y = size_y >> 1;
	if (size_y == 0) size_y = 1;
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

    glColor3f(1.0, 1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    xOff = 0.0;
    for (i = 0; i < 2; i ++ ){

	glLoadIdentity();
	glTranslatef(xOff, 0.0, -10.0);
	defineLevels(i, GL_COMPILE, ONE_DLIST, 0);
	drawLevels();

	glLoadIdentity();
	glTranslatef(xOff, TEXSIZE_0_Y+10.0, -10.0);
	defineLevels(i, GL_COMPILE, MANY_DLIST, 0);
	drawLevels();

	glLoadIdentity();
	glTranslatef(xOff, 2*(TEXSIZE_0_Y+10.0), -10.0);
	defineLevels(i, GL_COMPILE, MANY_DLIST, 1);
	drawLevels();

	glLoadIdentity();
	glTranslatef(2*(TEXSIZE_0_X+15.0) + xOff, 0.0, -10.0);
	defineLevels(i, GL_COMPILE_AND_EXECUTE, ONE_DLIST, 0);
	drawLevels();

	glLoadIdentity();
	glTranslatef(2*(TEXSIZE_0_X+15.0) + xOff, TEXSIZE_0_Y+10.0,-10.0);
	defineLevels(i, GL_COMPILE_AND_EXECUTE, MANY_DLIST, 0);
	drawLevels();

	glLoadIdentity();
	glTranslatef(2*(TEXSIZE_0_X+15.0) + xOff, 2*(TEXSIZE_0_Y+10.0), -10.0);
	defineLevels(i, GL_COMPILE_AND_EXECUTE, MANY_DLIST, 1);
	drawLevels();

	xOff = 4 * (TEXSIZE_0_X + 15.0);
    }
}

static void drawLevels(void)
{
    GLint majer, size_x, size_y;

    majer = (TEXSIZE_0_X > TEXSIZE_0_Y) ? TEXSIZE_0_X : TEXSIZE_0_Y;
    size_x = TEXSIZE_0_X;
    size_y = TEXSIZE_0_Y;
    for (; majer > 0; majer >>= 1){
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(3.0, 3.0, 0.0); 

	glTexCoord2f(1.0, 0.0);
	glVertex3f(3.0 + size_x, 3.0, 0.0); 

	glTexCoord2f(1.0, 1.0);
	glVertex3f(3.0 + size_x, 3.0 + size_y, 0.0);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(3.0, 3.0 + size_y, 0.0); 
	glEnd();

	glTranslatef(size_x + 3.0, 0.0, 0.0);

	size_x = size_x >> 1;
	if (size_x == 0) size_x = 1;
	size_y = size_y >> 1;
	if (size_y == 0) size_y = 1;
    }
}

static void defineLevels(GLint firstLevel, GLenum dlistMode, 
					GLboolean oneDlist,GLint dlistIdStep)
{
    GLint majer, size_x, size_y, lod, i, j, dlistId, numDlist;
    GLubyte tplain[TEXSIZE_0_X*TEXSIZE_0_Y*COMPON], *t;

	/* defin im levels */
    if (firstLevel == 0){
        majer = (TEXSIZE_0_X > TEXSIZE_0_Y) ? TEXSIZE_0_X : TEXSIZE_0_Y;
        size_x = TEXSIZE_0_X;
        size_y = TEXSIZE_0_Y;
    }else{
        majer = ((TEXSIZE_0_X > TEXSIZE_0_Y) ? TEXSIZE_0_X : TEXSIZE_0_Y) >> 1;
        size_x = TEXSIZE_0_X >> 1;
        size_y = TEXSIZE_0_Y >> 1;
    }

    for (lod = firstLevel; majer > 0; majer >>= 2, lod += 2){

	for (i = 0, t = tplain; i < size_x*size_y; i ++, t += COMPON){
	    for ( j = 0; j < COMPON; j ++ )
		t[j] = color[lod % NUM_COLORS][j];
	}

	glTexImage2D(GL_TEXTURE_2D, lod, 4, size_x, size_y, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, tplain);
	size_x = size_x >> 2;
	if (size_x == 0) size_x = 1;
	size_y = size_y >> 2;
	if (size_y == 0) size_y = 1;
    }

	/* defin dl levels */
    if (firstLevel == 0){
        majer = ((TEXSIZE_0_X > TEXSIZE_0_Y) ? TEXSIZE_0_X : TEXSIZE_0_Y) >> 1;
        size_x = TEXSIZE_0_X >> 1;
        size_y = TEXSIZE_0_Y >> 1;
	firstLevel = 1;
    }else{
        majer = (TEXSIZE_0_X > TEXSIZE_0_Y) ? TEXSIZE_0_X : TEXSIZE_0_Y;
        size_x = TEXSIZE_0_X;
        size_y = TEXSIZE_0_Y;
	firstLevel = 0;
    }

    dlistId = 101;
    if (oneDlist){
	glNewList(dlistId, dlistMode);
	numDlist = 1;
    }else
	numDlist = 0;
    
    for (lod = firstLevel; majer > 0; majer >>= 2, lod += 2){

	if (! oneDlist){
	   glNewList(dlistId, dlistMode);
	   numDlist ++;
	}

	glTexImage2D(GL_TEXTURE_2D, lod, 4, size_x, size_y, 0,
				GL_RGBA, GL_UNSIGNED_INT, tpattern[lod]);
	if (! oneDlist) glEndList();
	dlistId += dlistIdStep;

	size_x = size_x >> 2;
	if (size_x == 0) size_x = 1;
	size_y = size_y >> 2;
	if (size_y == 0) size_y = 1;
    }

    if (oneDlist) glEndList();

    if (dlistMode == GL_COMPILE){
	dlistId = 101;
	for (i = 0; i < numDlist; i++ ){
	    glCallList(dlistId);
	    dlistId += dlistIdStep;
	}
    }

}

CLEANUP(txdlim4_3)
{
    glDisable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                                GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    ogLibSetDefaultTextures();

    glDeleteLists(101, 10);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0, 0.0, 0.0, 0.0);
}

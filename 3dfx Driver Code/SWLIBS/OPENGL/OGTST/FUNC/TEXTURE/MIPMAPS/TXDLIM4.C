/*
 * txdlim4.c - test combination of the display list and immediate mode 4 
 * components texture
 */

#include "ogtst.h"       /* include test environment */


#define COMPON	4

#define TEXSIZE_0_X	256
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

static GLubyte t0[TEXSIZE_0_X * TEXSIZE_0_Y * COMPON];
static GLubyte t1[TEXSIZE_1_X * TEXSIZE_1_Y * COMPON];
static GLubyte t2[TEXSIZE_2_X * TEXSIZE_2_Y * COMPON];
static GLubyte t3[TEXSIZE_3_X * TEXSIZE_3_Y * COMPON];
static GLubyte t4[TEXSIZE_4_X * TEXSIZE_4_Y * COMPON];
static GLubyte t5[TEXSIZE_5_X * TEXSIZE_5_Y * COMPON];
static GLubyte t6[TEXSIZE_6_X * TEXSIZE_6_Y * COMPON];
static GLubyte t7[TEXSIZE_7_X * TEXSIZE_7_Y * COMPON];
static GLubyte t8[TEXSIZE_8_X * TEXSIZE_8_Y * COMPON];

static GLubyte *tpattern[13] = {t0, t1, t2, t3, t4, t5, t6, t7, t8};

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

static void drawLevels(void);
static void defineImLevels(GLint firstLevel);

/*ARGSUSED*/
TESTMOD(txdlim4)
{
    GLint width, height, lod, majer, size_x, size_y, i, j;
    GLubyte *t;

	/* define dlist texture images */
    glNewList(101, GL_COMPILE);
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
	glTexImage2D(GL_TEXTURE_2D, lod, 4, size_x, size_y, 
				0, GL_RGBA, GL_UNSIGNED_BYTE, tpattern[lod]);
	size_x = size_x >> 1;
	if (size_x == 0) size_x = 1;
	size_y = size_y >> 1;
	if (size_y == 0) size_y = 1;
    }
    glEndList();


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

    glColor3f(1.0, 1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);
    glCallList(101);
    drawLevels();

    
    glLoadIdentity();
    glTranslatef(0.0, TEXSIZE_0_Y + 10.0, -10.0);
    defineImLevels(0);
    drawLevels();


    glLoadIdentity();
    glTranslatef(0.0, 2 * (TEXSIZE_0_Y + 10.0), -10.0);
    glCallList(101);
    defineImLevels(1);
    drawLevels();
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
	glVertex3f(10.0, 10.0, 0.0); 

	glTexCoord2f(1.0, 0.0);
	glVertex3f(10.0 + size_x, 10.0, 0.0); 

	glTexCoord2f(1.0, 1.0);
	glVertex3f(10.0 + size_x, 10.0 + size_y, 0.0);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(10.0, 10.0 + size_y, 0.0); 
	glEnd();

	glTranslatef(size_x + 5.0, 0.0, 0.0);

	size_x = size_x >> 1;
	if (size_x == 0) size_x = 1;
	size_y = size_y >> 1;
	if (size_y == 0) size_y = 1;
    }
    glFinish(); /* temp workaround for RE3 */
}

static void defineImLevels(GLint firstLevel)
{
    GLint majer, size_x, size_y, lod, i, j;
    GLuint tplain[TEXSIZE_0_X*TEXSIZE_0_Y*COMPON], *t;

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
				GL_RGBA, GL_UNSIGNED_INT, tplain);
	size_x = size_x >> 2;
	if (size_x == 0) size_x = 1;
	size_y = size_y >> 2;
	if (size_y == 0) size_y = 1;
    }
}

CLEANUP(txdlim4)
{
    glDisable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                                GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    ogLibSetDefaultTextures();

    glDeleteLists(101, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0, 0.0, 0.0, 0.0);
}

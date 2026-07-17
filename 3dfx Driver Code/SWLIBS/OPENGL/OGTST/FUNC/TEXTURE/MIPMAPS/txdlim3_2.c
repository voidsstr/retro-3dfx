/*
 * txdlim3_2.c - test combination of the display lists and immediate mode 43
 * components texture
 */

#include "ogtst.h"       /* include test environment */


#define COMPON	3

#define TEXSIZE_0_X	128
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

#define TEXSIZE_0_X_4	(TEXSIZE_0_X >> 2)

static GLubyte t0[TEXSIZE_0_X * TEXSIZE_0_Y * COMPON];
static GLubyte t1[TEXSIZE_1_X * TEXSIZE_1_Y * COMPON];
static GLubyte t2[TEXSIZE_2_X * TEXSIZE_2_Y * COMPON];
static GLubyte t3[TEXSIZE_3_X * TEXSIZE_3_Y * COMPON];
static GLubyte t4[TEXSIZE_4_X * TEXSIZE_4_Y * COMPON];
static GLubyte t5[TEXSIZE_5_X * TEXSIZE_5_Y * COMPON];
static GLubyte t6[TEXSIZE_6_X * TEXSIZE_6_Y * COMPON];
static GLubyte t7[TEXSIZE_7_X * TEXSIZE_7_Y * COMPON];
static GLubyte t8[TEXSIZE_8_X * TEXSIZE_8_Y * COMPON];

static GLubyte *texture[13] = {t0, t1, t2, t3, t4, t5, t6, t7, t8};

#define NUM_COLORS	6
#define ONE 0xffffffff

static GLuint color[NUM_COLORS][COMPON] = {{ONE, 0, 0}, {0, ONE, 0},
  {0, 0, ONE}, {ONE, ONE, 0}, {0, ONE, ONE}, {ONE, 0, ONE}};

static void drawLevels(void);

/*ARGSUSED*/
TESTMOD(txdlim3_2)
{
    GLint width, height, lod, majer, size_x, size_y, i, j;
    GLubyte *t;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* define dlist texture images for ALL levels */
    glNewList(101, GL_COMPILE);
    majer = (TEXSIZE_0_X > TEXSIZE_0_Y) ? TEXSIZE_0_X : TEXSIZE_0_Y;
    size_x = TEXSIZE_0_X;
    size_y = TEXSIZE_0_Y;
    for ( lod = 0; majer > 0; majer >>= 1, lod ++ ){
        	/* init texture */
	for (i = 0, t = texture[lod]; i < size_x * size_y; i ++, t += COMPON){
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
	glTexImage2D(GL_TEXTURE_2D, lod, 3, size_x, size_y, 
				0, GL_RGB, GL_UNSIGNED_BYTE, texture[lod]);
	size_x = size_x >> 1;
	if (size_x == 0) size_x = 1;
	size_y = size_y >> 1;
	if (size_y == 0) size_y = 1;
    }
    glEndList();

	/* define dlist texture images for NOT 0 levels */
    glNewList(102, GL_COMPILE);
    majer = ((TEXSIZE_0_X > TEXSIZE_0_Y) ? TEXSIZE_0_X : TEXSIZE_0_Y) >> 1;
    size_x = TEXSIZE_0_X >> 1;
    size_y = TEXSIZE_0_Y >> 1;
    for ( lod = 1; majer > 0; majer >>= 1, lod ++ ){
        for (i = 0, t = texture[lod]; i < size_x*size_y; i ++, t += COMPON){
            for ( j = 0; j < COMPON; j ++ )
                t[j] = color[lod % NUM_COLORS][j];
        }

        glTexImage2D(GL_TEXTURE_2D, lod, 3, size_x, size_y, 0,
                                GL_RGB, GL_UNSIGNED_BYTE, texture[lod]);
        size_x = size_x >> 1;
        if (size_x == 0) size_x = 1;
        size_y = size_y >> 1;
        if (size_y == 0) size_y = 1;
    }
    glEndList();

        /* define im texture image for 0 level */
    for (i = 0; i <TEXSIZE_0_Y; i ++){
      for(j = 0; j < TEXSIZE_0_X; j ++){
        if (j < TEXSIZE_0_X_4){
           t0[i*TEXSIZE_0_X*COMPON + j*COMPON] = ONE;
           t0[i*TEXSIZE_0_X*COMPON + j*COMPON + 1] = ONE;
           t0[i*TEXSIZE_0_X*COMPON + j*COMPON + 2] = 0;
        }else if (j < (TEXSIZE_0_X_4*2)){
           t0[i*TEXSIZE_0_X*COMPON + j*COMPON] = ONE;
           t0[i*TEXSIZE_0_X*COMPON + j*COMPON + 1] = 0;
           t0[i*TEXSIZE_0_X*COMPON + j*COMPON + 2] = 0;
        }else if (j < (TEXSIZE_0_X_4*3)){
           t0[i*TEXSIZE_0_X*COMPON + j*COMPON] = 0;
           t0[i*TEXSIZE_0_X*COMPON + j*COMPON + 1] = ONE;
           t0[i*TEXSIZE_0_X*COMPON + j*COMPON + 2] = 0;
        }else{
           t0[i*TEXSIZE_0_X*COMPON + j*COMPON] = 0;
           t0[i*TEXSIZE_0_X*COMPON + j*COMPON + 1] = 0;
           t0[i*TEXSIZE_0_X*COMPON + j*COMPON + 2] = ONE;
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

    glColor3f(1.0, 1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);


    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);
    glCallList(101);
    drawLevels();
    
    glLoadIdentity();
    glTranslatef(0.0, TEXSIZE_0_Y + 30.0, -10.0);
    glCallList(102);
    drawLevels();

    glLoadIdentity();
    glTranslatef(0.0, 2 * (TEXSIZE_0_Y + 30.0), -10.0);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEXSIZE_0_X, TEXSIZE_0_Y, 0, 
						GL_RGB, GL_UNSIGNED_BYTE, t0);
    drawLevels();


    glLoadIdentity();
    glTranslatef(300.0, 0.0, -10.0);
    glCallList(101);
    drawLevels();
   
    glLoadIdentity();
    glTranslatef(300.0, TEXSIZE_0_Y + 30.0, -10.0);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEXSIZE_0_X, TEXSIZE_0_Y, 0, 
						GL_RGB, GL_UNSIGNED_BYTE, t0);
    drawLevels();

    glLoadIdentity();
    glTranslatef(300.0, 2 * (TEXSIZE_0_Y + 30.0), -10.0);
    glCallList(102);
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


CLEANUP(txdlim3_2)
{
    glDisable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                                GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    ogLibSetDefaultTextures();

    glDeleteLists(101, 2);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0, 0.0, 0.0, 0.0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

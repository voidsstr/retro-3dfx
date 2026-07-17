/*
 * unsign34.c - test unsigned textures: number of source components is 3,
 *                number of destination components is 4
 */

#include "ogtst.h"       /* include test environment */

#define NUM_TESTS 16

#define BYTE_ONE	0xff
#define SHORT_ONE	0xffff
#define INT_ONE		0xffffffff

#define COMPON	3
#define TEXSIZE	8
#define TEXSIZE_4	(TEXSIZE >> 2)

/*ARGSUSED*/
TESTMOD(unsign34)
{
    GLint width, height, doDlist, flag, i, j;

    GLubyte tubyte[TEXSIZE * TEXSIZE * COMPON];
    GLushort tushort[TEXSIZE * TEXSIZE * COMPON];
    GLuint tuint[TEXSIZE * TEXSIZE * COMPON];

	/* init texture */

	/* unsigned byte */
    for (i = 0; i <TEXSIZE; i ++){
      for(j=0; j<TEXSIZE; j++){
        if (((j + i*TEXSIZE_4) % TEXSIZE) < TEXSIZE_4){
           tubyte[i*TEXSIZE*COMPON + j*COMPON] = BYTE_ONE;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 1] = BYTE_ONE;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 2] = BYTE_ONE;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*2)){
           tubyte[i*TEXSIZE*COMPON + j*COMPON] = BYTE_ONE;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*3)){
           tubyte[i*TEXSIZE*COMPON + j*COMPON] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 1] = BYTE_ONE;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
        }else{
           tubyte[i*TEXSIZE*COMPON + j*COMPON] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 2] = BYTE_ONE;
        }
      }
    }

	/* unsigned short */
    for (i = 0; i <TEXSIZE; i ++){
      for(j=0; j<TEXSIZE; j++){
        if (((j + i*TEXSIZE_4) % TEXSIZE) < TEXSIZE_4){
           tushort[i*TEXSIZE*COMPON + j*COMPON] = 0;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 2] = SHORT_ONE;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*2)){
           tushort[i*TEXSIZE*COMPON + j*COMPON] = SHORT_ONE;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 1] = SHORT_ONE;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 2] = SHORT_ONE;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*3)){
           tushort[i*TEXSIZE*COMPON + j*COMPON] = SHORT_ONE;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
        }else{
           tushort[i*TEXSIZE*COMPON + j*COMPON] = 0;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 1] = SHORT_ONE;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
        }
      }
    }

	/* unsigned int */
    for (i = 0; i <TEXSIZE; i ++){
      for(j=0; j<TEXSIZE; j++){
        if (((j + i*TEXSIZE_4) % TEXSIZE) < TEXSIZE_4){
           tuint[i*TEXSIZE*COMPON + j*COMPON] = 0;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 1] = INT_ONE;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*2)){
           tuint[i*TEXSIZE*COMPON + j*COMPON] = 0;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 2] = INT_ONE;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*3)){
           tuint[i*TEXSIZE*COMPON + j*COMPON] = INT_ONE;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 1] = INT_ONE;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 2] = INT_ONE;
        }else{
           tuint[i*TEXSIZE*COMPON + j*COMPON] = INT_ONE;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
        }
      }
    }

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    width = ogEnvQuery(OG_XWSIZE) - 1;
    height = ogEnvQuery(OG_YWSIZE) - 1;
    glOrtho(-0.5, width + 0.5, -0.5, height + 0.5, 0.01, 1000.0);

    glClearColor(0.25, 0.25, 0.25, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); 
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -10.0);

    glColor3f(1.0, 1.0, 1.0);

    for (i = 0, j = 0; i < (TEXSIZE*TEXSIZE*COMPON); i = i + COMPON, j++){
        ogEnvLog(5, "ubyte(%i): 0x%8x\n", j, ogLib4Ub("DECAL", tubyte[i], tubyte[i+1], tubyte[i+2], 0xff, 1.0, 1.0, 1.0, 1.0));
    }
    for (i = 0, j = 0; i < (TEXSIZE*TEXSIZE*COMPON); i = i + COMPON, j++){
        ogEnvLog(5, "ushort(%i): 0x%8x\n", j, ogLib4Us("DECAL", tushort[i], tushort[i+1], tushort[i+2], 0xffff, 1.0, 1.0, 1.0, 1.0));
    }
    for (i = 0, j = 0; i < (TEXSIZE*TEXSIZE*COMPON); i = i + COMPON, j++){
        ogEnvLog(5, "uint(%i): 0x%8x\n", j, ogLib4Ui("DECAL", tuint[i], tuint[i+1], tuint[i+2], 0xffffffff, 1.0, 1.0, 1.0, 1.0));
    }
    
    for (i = 0; i != NUM_TESTS; ){
	flag = (i > (NUM_TESTS >> 1)) ? 1 : 0;
	doDlist = (i + flag) % 2;

	if (doDlist)
	    glNewList(1, GL_COMPILE);

	switch( i % 8 ){
	  case 0:
	  case 6:
	    glTexImage2D(GL_TEXTURE_2D, 0, 4, TEXSIZE, TEXSIZE, 0, 
					GL_RGB, GL_UNSIGNED_BYTE, tubyte);
	    break;

	  case 1:
	  case 4:
	    glTexImage2D(GL_TEXTURE_2D, 0, 4, TEXSIZE, TEXSIZE, 0, 
					GL_RGB, GL_UNSIGNED_SHORT, tushort);
	    break;

	  case 2:
	  case 5:
	    glTexImage2D(GL_TEXTURE_2D, 0, 4, TEXSIZE, TEXSIZE, 0, 
					GL_RGB, GL_UNSIGNED_INT, tuint);
	    break;
	  default:
	    glTexImage2D(GL_TEXTURE_2D, 0, 4, 0, TEXSIZE, 0, 
					GL_RGB, GL_UNSIGNED_INT, tuint);
	    break;
	}

	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0);
        glVertex2f(10.0, 10.0); 

	glTexCoord2f(1.0, 0.0);
        glVertex2f(100.0, 10.0); 

	glTexCoord2f(1.0, 1.0);
        glVertex2f(100.0, 100.0);

	glTexCoord2f(0.0, 1.0);
        glVertex2f(10.0, 100.0); 
	glEnd();

	if (doDlist){
	    glEndList();
	    glCallList(1);
	}

	i ++;

	if ((i % 4) == 0)
	    glTranslatef(-360.0, 120.0, 0.0);
	else
	    glTranslatef(120.0, 0, 0.0);

    }

}

CLEANUP(unsign34)
{
    glDisable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                                GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    ogLibSetDefaultTextures();

    glDeleteLists(1, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0, 0.0, 0.0, 0.0);
}

/*
 * unsign43.c - test unsigned textures: number of source components is 4,
 *                number of destination components is 3
 */

#include "ogtst.h"       /* include test environment */

#define NUM_TESTS 16

#define BYTE_ONE	0xff
#define SHORT_ONE	0xffff
#define INT_ONE		0xffffffff
#define BYTE_ALPHA_1	0x33
#define BYTE_ALPHA_2	0xaa
#define BYTE_ALPHA_3	0xcc
#define BYTE_ALPHA_4	0xff
#define SHORT_ALPHA_1	0x2222
#define SHORT_ALPHA_2	0x9999
#define SHORT_ALPHA_3	0xbbbb
#define SHORT_ALPHA_4	0xeeee
#define INT_ALPHA_1	0x11111111
#define INT_ALPHA_2	0x88888888
#define INT_ALPHA_3	0xaaaaaaaa
#define INT_ALPHA_4	0xdddddddd

#define COMPON	4
#define TEXSIZE	8
#define TEXSIZE_4	(TEXSIZE >> 2)

/*ARGSUSED*/
TESTMOD(unsign43)
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
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 3] = BYTE_ALPHA_1;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*2)){
           tubyte[i*TEXSIZE*COMPON + j*COMPON] = BYTE_ONE;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 3] = BYTE_ALPHA_2;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*3)){
           tubyte[i*TEXSIZE*COMPON + j*COMPON] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 1] = BYTE_ONE;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 3] = BYTE_ALPHA_3;
        }else{
           tubyte[i*TEXSIZE*COMPON + j*COMPON] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 2] = BYTE_ONE;
           tubyte[i*TEXSIZE*COMPON + j*COMPON + 3] = BYTE_ALPHA_4;
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
           tushort[i*TEXSIZE*COMPON + j*COMPON + 3] = SHORT_ALPHA_1;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*2)){
           tushort[i*TEXSIZE*COMPON + j*COMPON] = SHORT_ONE;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 1] = SHORT_ONE;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 2] = SHORT_ONE;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 3] = SHORT_ALPHA_2;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*3)){
           tushort[i*TEXSIZE*COMPON + j*COMPON] = SHORT_ONE;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 3] = SHORT_ALPHA_3;
        }else{
           tushort[i*TEXSIZE*COMPON + j*COMPON] = 0;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 1] = SHORT_ONE;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
           tushort[i*TEXSIZE*COMPON + j*COMPON + 3] = SHORT_ALPHA_4;
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
           tuint[i*TEXSIZE*COMPON + j*COMPON + 3] = INT_ALPHA_1;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*2)){
           tuint[i*TEXSIZE*COMPON + j*COMPON] = 0;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 2] = INT_ONE;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 3] = INT_ALPHA_2;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*3)){
           tuint[i*TEXSIZE*COMPON + j*COMPON] = INT_ONE;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 1] = INT_ONE;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 2] = INT_ONE;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 3] = INT_ALPHA_3;
        }else{
           tuint[i*TEXSIZE*COMPON + j*COMPON] = INT_ONE;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
           tuint[i*TEXSIZE*COMPON + j*COMPON + 3] = INT_ALPHA_4;
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
	ogEnvLog(5, "ubyte(%i): 0x%8x\n", j, ogLib3Ub("DECAL", tubyte[i], tubyte[i+1], tubyte[i+2], 1.0, 1.0, 1.0, 1.0));
    }
    for (i = 0, j = 0; i < (TEXSIZE*TEXSIZE*COMPON); i = i + COMPON, j++){
	ogEnvLog(5, "ushort(%i): 0x%8x\n", j, ogLib3Us("DECAL", tushort[i], tushort[i+1], tushort[i+2], 1.0, 1.0, 1.0, 1.0));
    }
    for (i = 0, j = 0; i < (TEXSIZE*TEXSIZE*COMPON); i = i + COMPON, j++){
	ogEnvLog(5, "uint(%i): 0x%8x\n", j, ogLib3Ui("DECAL", tuint[i], tuint[i+1], tuint[i+2], 1.0, 1.0, 1.0, 1.0));
    }
    
    for (i = 0; i != NUM_TESTS; ){
	flag = (i > (NUM_TESTS >> 1)) ? 1 : 0;
	doDlist = (i + flag) % 2;

        if (doDlist)
            glNewList(i + 1, GL_COMPILE);

	switch( i % 8 ){
	  case 0:
	  case 6:
	    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEXSIZE, TEXSIZE, 0, 
					GL_RGBA, GL_UNSIGNED_BYTE, tubyte);
	    break;

	  case 1:
	  case 4:
	    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEXSIZE, TEXSIZE, 0, 
					GL_RGBA, GL_UNSIGNED_SHORT, tushort);
	    break;

	  case 2:
	  case 5:
	    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEXSIZE, TEXSIZE, 0, 
					GL_RGBA, GL_UNSIGNED_INT, tuint);
	    break;
	  default:
	    glTexImage2D(GL_TEXTURE_2D, 0, 3, 0, TEXSIZE, 0, 
					GL_RGBA, GL_UNSIGNED_INT, tuint);
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
	    glCallList(i + 1);
        }

	i ++;

	if ((i % 4) == 0)
	    glTranslatef(-360.0, 120.0, 0.0);
	else
	    glTranslatef(120.0, 0, 0.0);

    }

}

CLEANUP(unsign43)
{
    glDisable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
						GL_NEAREST_MIPMAP_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    ogLibSetDefaultTextures();

    glDeleteLists(1, NUM_TESTS);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0, 0.0, 0.0, 0.0);
}

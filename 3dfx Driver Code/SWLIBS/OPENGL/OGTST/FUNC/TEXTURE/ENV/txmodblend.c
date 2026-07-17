/*
 * txmodblend.c - test modulate and blend texture environment
 */

#include "ogtst.h"       /* include test environment */


#define BYTE_ONE	0xff
#define TEXSIZE		16

/*ARGSUSED*/
TESTMOD(txmodblend)
{
    GLint width, height, doDlist, flag, i, j;
    GLubyte tla[TEXSIZE * TEXSIZE * 2];
    GLfloat fcolor[4], lumin, delta, alpha;
    GLint icolor[4];
    GLenum dstFormat;

	/* init texture */
    delta = 1.0 / (TEXSIZE*TEXSIZE);
    lumin = 0;
    alpha = 1.0;
    for (i = 0; i < TEXSIZE * TEXSIZE; i ++){
	tla[2*i] = lumin * BYTE_ONE;
	tla[2*i + 1] = alpha * BYTE_ONE;
	lumin += delta;
	alpha -= delta;
    }

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    width = ogEnvQuery(OG_XWSIZE) - 1;
    height = ogEnvQuery(OG_YWSIZE) - 1;
    glOrtho(-0.5, width + 0.5, -0.5, height + 0.5, 0.01, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);

    glClearColor(0.25, 0.25, 0.25, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 

    glColor3f(1.0, 0.0, 0.0);

    for (i = 0; i < 2; i ++){

	dstFormat = (i % 2) + 1;
	glTexImage2D(GL_TEXTURE_2D, 0, dstFormat, TEXSIZE, TEXSIZE, 0, 
				GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, tla);
	flag = 0;
	for (j = 0; j < 12; j ++){

	    doDlist = (j + flag) % 2;
	    if (doDlist)
		glNewList(1, GL_COMPILE);

	    switch( j % 6 ){
	      case 0:
		break;

	      case 1:
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
		break;

	      case 2:
		fcolor[0] = 0.0;
		fcolor[1] = 1.0;
		fcolor[2] = 0.0;
		fcolor[3] = 1.0;
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, fcolor);
		break;

	      case 3:
		icolor[0] = 0xffffffff;
		icolor[1] = 0xffffffff;
		icolor[2] = 0x7fffffff;
		icolor[3] = 0x7fffffff;
		glTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, icolor);
		break;

	      case 4:
		fcolor[0] = -1.0;
		fcolor[1] = 1.0;
		fcolor[2] = 1.0;
		fcolor[3] = 1.0;
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, fcolor);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		break;

	      case 5:
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
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
	
	    if ( (j == 5) || (j == 11) ){
		glTranslatef(-500.0, 101.0, 0.0);

		    /* reset to default values */
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		icolor[0] = 0;
		icolor[1] = 0;
		icolor[2] = 0;
		icolor[3] = 0;
		glTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, icolor);
		flag = 1;

	    }else
		glTranslatef(101.0, 0, 0.0);
	}
    }
 
}

CLEANUP(txmodblend)
{
    GLfloat color[] = {0, 0, 0, 0};
    glDisable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                                GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
    ogLibSetDefaultTextures();

    glDeleteLists(1, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    ogLibSetDefaultColors();
}

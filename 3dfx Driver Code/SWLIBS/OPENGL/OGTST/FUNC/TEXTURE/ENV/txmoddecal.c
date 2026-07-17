/*
 * txmoddecal.c - test modulate and decal texture environment
 */

#include "ogtst.h"       /* include test environment */


#define TEXSIZE		16
#define TEXSIZE_4     (TEXSIZE >> 2)

#define MODE	(doDlist ? "DL" : "IM")

/*ARGSUSED*/
TESTMOD(txmoddecal)
{
    GLint width, height, doDlist, i, j, s, t;
    GLfloat alpha, dalpha;
    GLfloat tplain[TEXSIZE * TEXSIZE * 4], tpattern[TEXSIZE * TEXSIZE * 4];

    dalpha = 1.0 / (TEXSIZE*TEXSIZE);
    alpha = 0;
    for (i = 0; i < TEXSIZE * TEXSIZE * 4; i +=4){
	tplain[i] = 0;
	tplain[i + 1] = 0.5;
	tplain[i + 2] = 1.0;
	tplain[i + 3] = alpha;
	alpha += dalpha;
    }

    dalpha = 1.0 / (TEXSIZE*TEXSIZE);
    alpha = 0;
    for (i = 0; i < TEXSIZE; i ++){
      for(j = 0; j < TEXSIZE; j ++){
	s = j / TEXSIZE_4;
	t = i / TEXSIZE_4;
	switch((s + t) % 4){
	  case 0:
            tpattern[i*TEXSIZE*4 + j*4] = 1.0;
            tpattern[i*TEXSIZE*4 + j*4 + 1] = 1.0;
            tpattern[i*TEXSIZE*4 + j*4 + 2] = 0.0;
            tpattern[i*TEXSIZE*4 + j*4 + 3] = alpha;
	    break;
	  case 1:
            tpattern[i*TEXSIZE*4 + j*4] = 1.0;
            tpattern[i*TEXSIZE*4 + j*4 + 1] = 0.0;
            tpattern[i*TEXSIZE*4 + j*4 + 2] = 0.0;
            tpattern[i*TEXSIZE*4 + j*4 + 3] = alpha;
	    break;
	  case 2:
            tpattern[i*TEXSIZE*4 + j*4] = 0.0;
            tpattern[i*TEXSIZE*4 + j*4 + 1] = 1.0;
            tpattern[i*TEXSIZE*4 + j*4 + 2] = 0.0;
            tpattern[i*TEXSIZE*4 + j*4 + 3] = alpha;
	    break;
	  case 3:
	    tpattern[i*TEXSIZE*4 + j*4] = 0.0;
	    tpattern[i*TEXSIZE*4 + j*4 + 1] = 0.0;
	    tpattern[i*TEXSIZE*4 + j*4 + 2] = 1.0;
	    tpattern[i*TEXSIZE*4 + j*4 + 3] = alpha;
	    break;
        }

	alpha += dalpha;
      }
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

    glColor3f(1.0, 0.5, 0.0);

    for (j = 0; j < 2; j ++){

      ogEnvLog(OG_LGENERAL,"Env: MODULATE\n");

      for (i = 0; i < 10; i ++){

	doDlist = (i + j) % 2;
	if (doDlist)
	    glNewList(1, GL_COMPILE);

	switch( i % 10 ){
	  case 0:
	    ogEnvLog(OG_LGENERAL,"%d %s Texture: plain RGB\n", i, MODE);
	    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEXSIZE, TEXSIZE, 0, 
				GL_RGBA, GL_FLOAT, tplain);
	    break;

	  case 1:
	    ogEnvLog(OG_LGENERAL,"%d %s Texture: pattern RGBA\n", i, MODE);
	    glTexImage2D(GL_TEXTURE_2D, 0, 4, TEXSIZE, TEXSIZE, 0, 
				GL_RGBA, GL_FLOAT, tpattern);
	    break;

	  case 2:
	    ogEnvLog(OG_LGENERAL,"%d %s Env: DECAL\n", i, MODE);
	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	    break;

	  case 3:
	    ogEnvLog(OG_LGENERAL,"%d %s Texture: plain RGBA\n", i, MODE);
	    glTexImage2D(GL_TEXTURE_2D, 0, 4, TEXSIZE, TEXSIZE, 0, 
				GL_RGBA, GL_FLOAT, tplain);
	    break;

	  case 4:
	    ogEnvLog(OG_LGENERAL,"%d %s Texture: pattern RGB\n", i, MODE);
	    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEXSIZE, TEXSIZE, 0, 
				GL_RGBA, GL_FLOAT, tpattern);
	    break;

	  case 5:
	    ogEnvLog(OG_LGENERAL,"%d %s Env: MODULATE\n", i, MODE);
	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	    break;

	  case 6:
	    ogEnvLog(OG_LGENERAL,"%d %s Texture: plain RGBA\n", i, MODE);
	    glTexImage2D(GL_TEXTURE_2D, 0, 4, TEXSIZE, TEXSIZE, 0, 
				GL_RGBA, GL_FLOAT, tplain);
	    break;

	  case 7:
	    ogEnvLog(OG_LGENERAL,"%d %s Env: DECAL\n", i, MODE);
	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	    break;

	  case 8:
	    ogEnvLog(OG_LGENERAL,"%d %s Texture: plain RGB\n", i, MODE);
	    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEXSIZE, TEXSIZE, 0, 
				GL_RGBA, GL_FLOAT, tplain);
	    break;

	  case 9:
	    ogEnvLog(OG_LGENERAL,"%d %s Env: MODULATE\n", i, MODE);
	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
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

	if ((i == 4) || (i == 9))
	    glTranslatef(-400.0, 100.0, 0.0);
	else
	    glTranslatef(103.0, 0.0, 0.0);

      }

	/* reset to the default value */
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
}

CLEANUP(txmoddecal)
{
    glDisable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                                GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    ogLibSetDefaultTextures();

    glDeleteLists(1, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    ogLibSetDefaultColors();
}

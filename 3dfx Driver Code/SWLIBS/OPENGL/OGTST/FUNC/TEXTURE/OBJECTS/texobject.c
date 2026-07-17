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

/* texobject.c - $Revision: 2$
 *
 * Exercise these things using texture objects:
 *
 */
#include <stdio.h>
#include "ogtst.h"
#include "GL/glu.h"

#ifdef GL_VERSION_1_1 
static int numtex = 16;
static int ssize = 64, tsize = 64;
static GLenum texfmt = GL_RGBA;
static GLenum textype = GL_UNSIGNED_BYTE;
static int winxsize, winysize;

#define NEVER_MIPMAP 0
#define TOGGLE_MIPMAP 1
#define ALWAYS_MIPMAP 2
static int mipmapctl = NEVER_MIPMAP;
static int subtexture = GL_FALSE;

static unsigned char fragcolors[][3] = {
    { 0xff, 0xff, 0xff },
    { 0xff, 0xff, 0x00 },
    { 0x00, 0xff, 0x00 },
    { 0xff, 0x00, 0xff },
};

static unsigned char solids[2][3] = {
    { 0x00, 0x00, 0x00 },
    { 0xff, 0xff, 0xff },
};

static unsigned char white[] = { 0xff, 0xff, 0xff, 0xff };
static unsigned char black[] = { 0x00, 0x00, 0x00, 0xff };
static unsigned char gray[]  = { 0x80, 0x80, 0x80, 0xff };

/**************************************************************************/

static int
getmaxlod(int ssize, int tsize)
{
    int i=0;

    while (ssize >= 1 || tsize >= 1) {
	if (ssize == 1 && tsize == 1) {
	    break;
	} else {
	    if (ssize > 1) ssize >>= 1;
	    if (tsize > 1) tsize >>= 1;
	}
	i++;
    }
    return i;
}

static void
define_levels_nofilter( int labelnum, int mipmap)
{
    char *tex, label[20];
    int ssz, tsz, maxlod, i=0;

    sprintf(label, "%3d", labelnum);
    maxlod = getmaxlod(ssize, tsize);
    for (i=0; i <= maxlod; i++) {
	ssz = ssize >> i; if (ssz < 1) ssz = 1;
	tsz = tsize >> i; if (tsz < 1) tsz = 1;

	if (i < 1) {
	    tex = ogLibGenLabelTex(ssz, tsz, GL_RGB, GL_UNSIGNED_BYTE,
                                   white, solids[i%2], label);
	} else {
	    tex = ogLibGenSolidTex(ssz, tsz, GL_RGB, GL_UNSIGNED_BYTE,
			      solids[i%2]);
	}
	if (subtexture) {
	    glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA8, ssz, tsz, 0,
			 GL_RGB, GL_UNSIGNED_BYTE, NULL);
	    glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, ssz, tsz,
			       GL_RGB, GL_UNSIGNED_BYTE, tex);
	} else {
	    glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA8, ssz, tsz, 0,
			 GL_RGB, GL_UNSIGNED_BYTE, tex);
	}
	ogLibFree(tex);
	if (!mipmap) break;
    }
}

static void
define_filter( int mipmap )
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (mipmap) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
    } else {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR);
    }
}

static void
define_levels( int labelnum, int mipmap )
{
    define_filter( mipmap );
    define_levels_nofilter( labelnum, mipmap );
}

static int
interp_mipmapctl(int i)
{
    switch (mipmapctl) {
      case NEVER_MIPMAP: return 0;
      case TOGGLE_MIPMAP: return (i & 0x1);
      case ALWAYS_MIPMAP: return 1;
      default:
	ogEnvLog(OG_LALWAYS,"unknown mipmap ctl mode\n");
    }
    return 0;
}

/**************************************************************************/

static int rows, cols;
static int vw, vh;

static void
prepare_window(int numtex, int nrows, int ncols)
{
    int size;

    if (numtex) {
	rows = cols = sqrt(numtex);
	if (rows * cols < numtex) cols++;
	if (rows * cols < numtex) rows++;
    } else {
	rows = nrows;
	cols = ncols;
    }
    vw = winxsize / (float)cols;
    vh = winysize / (float)rows;
    size = (vw < vh ? vw : vh);
    vw = vh = size;
    
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3ubv(fragcolors[0]);
}

static void
draw_primitive(int i, int mipmap)
{
    int x, y, vx, vy;
    
    x = i % cols;
    y = i / cols;
    vx = x*vw;
    vy = y*vh;
    glViewport(vx, vy, vw, vh);

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex3f(-2, -2, -2);
    glTexCoord2f(1, 0); glVertex3f( 2, -2, -2);
    glTexCoord2f(0, 1); glVertex3f(-2,  2, -2);
    glTexCoord2f(1, 1); glVertex3f( 2,  2, -2);
    glEnd();
    if (mipmap) {
	/* draw a part that shows the mipmaps */
	glBegin(GL_TRIANGLES);
	glTexCoord2f(0, 1); glVertex3f(-2,  1, -2);
	glTexCoord2f(1, 1); glVertex3f(38, 38.2, -40);
	glTexCoord2f(0, 0); glVertex3f(-2,  2, -2);
	glEnd();
    }
}

static GLint savevp[4];

static void
init(void)
{
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 90.0, 1.0, 0.1, 4000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -0.1);

    glClearColor(.5,.5,.5,1.);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glGetIntegerv(GL_VIEWPORT, savevp);
    winxsize = (ogEnvQuery(OG_XWSIZE));
    winysize = (ogEnvQuery(OG_YWSIZE));
}

static void
cleanup(void)
{
    float envcolor[] = { 0, 0, 0, 0 };
    
    glDisable(GL_TEXTURE_2D);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(1.0,1.0,1.0);
    glClearColor(0,0,0,0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		    GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envcolor);

    glTexCoord2f(0, 0);
    glViewport(savevp[0],savevp[1],savevp[2],savevp[3]);

    ogLibSetDefaultTextures();    
}
#endif

/**************************************************************************/

/*
 * Switch between immed textures.
 */
/*ARGSUSED*/
TESTMOD(texobj_immed)
{
#ifdef GL_VERSION_1_1
    if (IS_ONEONE()) {

       GLboolean mipmap;
       int i, j;

       init();
       prepare_window(0, 6, 8);
       for (j=0; j < 3; j++) {
	   switch (j) {
	     case 0: mipmapctl = NEVER_MIPMAP; break;
	     case 1: mipmapctl = TOGGLE_MIPMAP; break;
	     case 2: mipmapctl = ALWAYS_MIPMAP; break;
	   }
	   for (i=0; i < numtex; i++) {
	       mipmap = interp_mipmapctl(i);
	       define_levels(i, mipmap);
	       draw_primitive(j*numtex+i, mipmap);
	   }
       }
    }
#endif
}

CLEANUP(texobj_immed)
{
#ifdef GL_VERSION_1_1 
    cleanup();
#endif
}

/*
 * Switch between dlist textures.
 */

/*ARGSUSED*/
TESTMOD(texobj_dlist)
{
#ifdef GL_VERSION_1_1 
    if (IS_ONEONE()) {

       GLuint list;
       GLboolean mipmap;
       int i, j;

       init();
       prepare_window(0, 6, 8);
       for (j=0; j < 3; j++) {
	   switch (j) {
	     case 0: mipmapctl = NEVER_MIPMAP; break;
	     case 1: mipmapctl = TOGGLE_MIPMAP; break;
	     case 2: mipmapctl = ALWAYS_MIPMAP; break;
	   }
	   list = glGenLists(numtex);
	   for (i=0; i < numtex; i++) {
	       mipmap = interp_mipmapctl(i);
	       glNewList(list+i, GL_COMPILE);
	       define_levels(i, mipmap);
	       glEndList();
	   }
	   for (i=0; i < numtex; i++) {
	       mipmap = interp_mipmapctl(i);
	       glCallList(list+i);
	       draw_primitive(j*numtex+i, mipmap);
	   }
	   glDeleteLists(list, numtex);
       }
    }
#endif
}

CLEANUP(texobj_dlist)
{
#ifdef GL_VERSION_1_1
    cleanup();
#endif
}

/*
 * Switch between texture objects.
 */
/*ARGSUSED*/
TESTMOD(texobj_objects)
{
#ifdef GL_VERSION_1_1 
    if (IS_ONEONE()) {

       GLuint texnames[1024];
       GLboolean mipmap;
       int i, j;

       init();
       prepare_window(0, 6, 8);
       for (j=0; j < 3; j++) {
	   switch (j) {
	     case 0: mipmapctl = NEVER_MIPMAP; break;
	     case 1: mipmapctl = TOGGLE_MIPMAP; break;
	     case 2: mipmapctl = ALWAYS_MIPMAP; break;
	   }
	   glGenTextures(numtex, texnames);
	   for (i=0; i < numtex; i++) {
	       mipmap = interp_mipmapctl(i);
	       glBindTexture(GL_TEXTURE_2D, texnames[i]);
	       define_levels(i, mipmap);
	   }
	   for (i=0; i < numtex; i++) {
	       mipmap = interp_mipmapctl(i);
	       glBindTexture(GL_TEXTURE_2D, texnames[i]);
	       draw_primitive(j*numtex+i, mipmap);
	   }
	   glDeleteTextures(numtex, texnames);
       }
    }
#endif
}

CLEANUP(texobj_objects)
{
#ifdef GL_VERSION_1_1
    cleanup();
#endif
}

/*
 * Switch between texture objects that use subtexture.
 */
/*ARGSUSED*/
TESTMOD(texobj_subtexture)
{
#ifdef GL_VERSION_1_1 

    if (IS_ONEONE()) {

       GLuint texnames[1024];
       GLboolean mipmap;
       int i, j;

       init();
       prepare_window(0, 6, 8);
       subtexture = GL_TRUE;
       for (j=0; j < 3; j++) {
	   switch (j) {
	     case 0: mipmapctl = NEVER_MIPMAP; break;
	     case 1: mipmapctl = TOGGLE_MIPMAP; break;
	     case 2: mipmapctl = ALWAYS_MIPMAP; break;
	   }
	   glGenTextures(numtex, texnames);
	   for (i=0; i < numtex; i++) {
	       mipmap = interp_mipmapctl(i);
	       glBindTexture(GL_TEXTURE_2D, texnames[i]);
	       define_levels(i, mipmap);
	   }
	   for (i=0; i < numtex; i++) {
	       mipmap = interp_mipmapctl(i);
	       glBindTexture(GL_TEXTURE_2D, texnames[i]);
	       draw_primitive(j*numtex+i, mipmap);
	   }
	   glDeleteTextures(numtex, texnames);
       }
       subtexture = GL_FALSE;
    }
#endif
}

CLEANUP(texobj_subtexture)
{
#ifdef GL_VERSION_1_1
    cleanup();
#endif
}

/*ARGSUSED*/
TESTMOD(texobj_delete)
{
#ifdef GL_VERSION_1_1

    if (IS_ONEONE()) {

       GLuint texnames[1024];
       GLboolean mipmap;
       int numtex = 4;
       int i;

       init();
       mipmapctl = ALWAYS_MIPMAP;
       glBindTexture(GL_TEXTURE_2D, 0);
       define_levels(0, GL_TRUE);

       /* delete empties */
       glGenTextures(numtex, texnames);
       glDeleteTextures(numtex, texnames);

       prepare_window(0, 4, 8);

       /* delete while bound, before pool; should see 0, 0, 0, 1, 0, 2, etc. */
       glGenTextures(numtex, texnames);
       for (i=0; i < numtex; i++) {
	   mipmap = interp_mipmapctl(i);
	   glBindTexture(GL_TEXTURE_2D, texnames[i]);
	   define_levels(i, mipmap);
	   draw_primitive(i*2, mipmap);
	   glDeleteTextures(1, &texnames[i]);
	   draw_primitive(i*2+1, mipmap);
       }

       /* delete while bound, after pool; should see 0, 0, 0, 1, 0, 2, etc. */
       glGenTextures(numtex, texnames);
       for (i=0; i < numtex; i++) {
	   mipmap = interp_mipmapctl(i);
	   glBindTexture(GL_TEXTURE_2D, texnames[i]);
	   define_levels(i, mipmap);
	   glBindTexture(GL_TEXTURE_2D, texnames[i]);
	   draw_primitive(8+i*2, mipmap);
	   glDeleteTextures(1, &texnames[i]);
	   draw_primitive(8+i*2+1, mipmap);
       }

       /* delete before bound, before pool; should see 0, blank, 1, blank, etc. */
       glGenTextures(numtex, texnames);
       for (i=0; i < numtex; i++) {
	   mipmap = interp_mipmapctl(i);
	   glBindTexture(GL_TEXTURE_2D, texnames[i]);
	   define_levels(i, mipmap);
	   draw_primitive(16+i*2, mipmap);
	   glDeleteTextures(1, &texnames[i]);
	   glBindTexture(GL_TEXTURE_2D, texnames[i]);
	   /* bound to a new, undefined texture */
	   draw_primitive(16+i*2+1, mipmap);
       }
       glBindTexture(GL_TEXTURE_2D, 0);
       glDeleteTextures(numtex, texnames);

       /* delete before bound, after pool; should see 0, blank, 1, blank, etc.t */
       glGenTextures(numtex, texnames);
       for (i=0; i < numtex; i++) {
	   mipmap = interp_mipmapctl(i);
	   glBindTexture(GL_TEXTURE_2D, texnames[i]);
	   define_levels(i, mipmap);
	   glBindTexture(GL_TEXTURE_2D, texnames[i]);
	   draw_primitive(24+i*2, mipmap);
	   glDeleteTextures(1, &texnames[i]);
	   glBindTexture(GL_TEXTURE_2D, texnames[i]);
	   /* bound to a new, undefined texture */
	   draw_primitive(24+i*2+1, mipmap);
       }
       glBindTexture(GL_TEXTURE_2D, 0);
       glDeleteTextures(numtex, texnames);
    }
#endif
}

CLEANUP(texobj_delete)
{
#ifdef GL_VERSION_1_1
    cleanup();
#endif
}

/*ARGSUSED*/
TESTMOD(texobj_env)
{
#ifdef GL_VERSION_1_1

    if (IS_ONEONE()) {

       GLuint texnames[64];
       char *tex, label[20];
       int i;
       int numtex = 64;
       GLenum ifmt;
       float envred[] = { 1, 0, 0, 1 };
       float envgreen[] = { 0, 0, 1, 1 };
       unsigned char white_half_alpha[] = { 0xff, 0xff, 0xff, 0x80 };

       init();
       glGenTextures(numtex, texnames);
       for (i=0; i < numtex; i++) {
	   glBindTexture(GL_TEXTURE_2D, texnames[i]);
	   sprintf(label, "%3d", i);
	   tex = ogLibGenLabelTex(ssize, tsize, texfmt, textype,
				  white_half_alpha, black, label);
	   /*
	   ** Try 3-component in upper half, 4-component in bottom half of
	   ** window.  The only difference will be for GL_DECAL, see (*) below.
	   */
	   if (i < numtex/2) {
	       /* ifmt = 4; */
	       ifmt = GL_RGBA8;
	   } else {
	       /* ifmt = 3; */
	       ifmt = GL_RGB8;
	   }
	   glTexImage2D(GL_TEXTURE_2D, 0, ifmt, ssize, tsize, 0,
			texfmt, textype, tex);
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	   ogLibFree(tex);
       }
       prepare_window(numtex, 0, 0);
       for (i=0; i < numtex; i++) {
	   glBindTexture(GL_TEXTURE_2D, texnames[i]);
	   switch (i % 4) {
	     case 0:
	       /* blue label, black background */
	       glColor4f(0,0,1,1);
	       glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	       break;
	     case 1:
	       /* light-blue label (*white label if top half), black background */
	       glColor4f(0,0,1,1);
	       glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	       break;
	     case 2:
	       /* red label, white background */
	       glColor4f(1,1,1,1);
	       glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
	       glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envred);
	       break;
	     case 3:
	       /* yellow label, white background */
	       glColor4f(1,1,1,1);
	       glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
	       glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envgreen);
	       break;
	   }
	   draw_primitive(i, GL_FALSE);
       }
       glDeleteTextures(numtex, texnames);
    }
#endif
}

CLEANUP(texobj_env)
{
#ifdef GL_VERSION_1_1
    cleanup();
#endif
}

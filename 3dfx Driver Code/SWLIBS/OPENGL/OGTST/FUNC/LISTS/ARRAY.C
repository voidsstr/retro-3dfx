/************************************************************************
* 		 Copyright (C) 1989, Silicon Graphics, Inc.	      *
*								      *
* These coded instructions, statements, and computer programs  contain *
* unpublished  proprietary  information of Silicon Graphics, Inc., and *
* are protected by Federal copyright law.  They  may  not be disclosed *
* to  third  parties  or copied or duplicated in any form, in whole or *
* in part, without the prior written consent of Silicon Graphics, Inc. *
************************************************************************/

/* array.c - $Revision: 2$ */

/*
** This program tests glCallLists() with various dlist array types
*/
#include "ogtst.h"	/* include test environment */
#include <assert.h>     /* allow assertions */
#include <limits.h>     /* for max int */

enum {MAX_LISTS = 200}; /* initialize arrays */
GLint MaxLists = MAX_LISTS; /* adjust to screen width */

/*
** This function returns an array containing a list of <n> valid display
** list numbers, each number of type <type>
*/
GLuint array[MAX_LISTS];

int typesize[] = {
    8, /* GL_BYTE */
    8, /* GL_UNSIGNED_BYTE */
    16, /* GL_SHORT */
    16, /* GL_UNSIGNED_SHORT */
    32, /* GL_INT */
    32, /* GL_UNSIGNED_INT */
    24, /* GL_FLOAT */ /* to avoid conversions problems */
    8, /* GL_2_BYTES */
    24, /* GL_3_BYTES */
    32, /* GL_4_BYTES */
};


#define SET_ARRAY(type)                            \
{                                                  \
    type *tmp; /* to get rid of bogus warnings */  \
    tmp = (type *)offsets;	                   \
    *tmp = (type)((type)dlists[i] - (type)*base);  \
    ogEnvLog(2, "offset: %d: %d\n", i, *tmp);      \
    tmp++; /* casts prevent unsigned promotion */  \
    offsets = (GLvoid *)tmp;                       \
}

/* used by both make_lists and delete_lists */
GLuint dlists[MAX_LISTS]; 

/*
** Create a display lists for each base + list combination
*/
void make_lists(GLsizei n, GLenum type, GLuint *base, GLvoid *offsets, 
		GLuint wid, GLuint ht, GLuint color)
{
    /* list of dlist names */
    GLubyte r,g,b,a;
    GLuint min, max; /* minimum and maximum dlist name */
    GLint i;
    GLint index;
    GLint maxplus, maxminus; /* range of a given signed value */

    r = (color >> 24) & 0xff;
    g = (color >> 16) & 0xff;
    b = (color >>  8) & 0xff;
    a = color & 0xff;

    assert(n <= MaxLists);

    index = type - GL_BYTE; /* convert to zero-based index */

    for(i = 0; i < n; i++) { /* write an array of dlists */
	do
            dlists[i] = ogLibBitRand(typesize[index]);
        while (!dlists[i]);
	glNewList(dlists[i], GL_COMPILE);
	glColor4ub(r, g, b, a); /* to catch dlists left from previous tests */
	glRecti(1, 1, wid + 1, ht + 1);
	glTranslatef(wid + 2,  ht + 2, 0);
	glEndList();
    }
    /*
    ** Find min and max boundaries; used to generate
    ** a reasonable base value.
    */
    min = max = dlists[0];
    for (i = 1; i < n; i++) {
        if(min > dlists[i])
            min = dlists[i];
        else if(max < dlists[i])
            max = dlists[i];
    }
    /* generate base */

    switch(type) {
    case GL_BYTE: /* signed values */
    case GL_SHORT:
    case GL_INT:
    case GL_FLOAT:
	maxminus = maxplus = 1 << (typesize[index] - 1);
	maxplus -= 1;
	*base = ogLibIntRand(max - maxplus, min + maxminus);
	break;
    case GL_2_BYTES: /* GL_*_BYTES unsigned ? */
    case GL_3_BYTES:
    case GL_4_BYTES:
    case GL_UNSIGNED_BYTE: /* unsigned values */
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
	*base = ogLibIntRand(1, min); /* all offsets must be positive */
	break;
    }

    /* generate offset list */

    for(i = 0; i < n; i++) {
	switch(type) {
	case GL_BYTE:
	    SET_ARRAY(GLbyte);
	    break;
	case GL_UNSIGNED_BYTE:
	    SET_ARRAY(GLubyte);
	    break;
	case GL_SHORT:
	    SET_ARRAY(GLshort);
	    break;
	case GL_2_BYTES:
	{
	    GLuint tmp;
	    GLubyte *tptr;
	    tptr = offsets;
	    tmp = dlists[i] - *base;
	    *tptr++ = (GLubyte)((tmp >> 8) & 0xff);
	    *tptr++ = (GLubyte)(tmp & 0xff);
	    offsets = (GLvoid *)tptr;
	}
	    break;
	case GL_UNSIGNED_SHORT:
	    SET_ARRAY(GLushort);
	    break;
	case GL_INT:
	    SET_ARRAY(GLuint);
	    break;
	case GL_4_BYTES:
	{
	    GLuint tmp;
	    GLubyte *tptr;
	    tptr = offsets;
	    tmp = dlists[i] - *base;
	    *tptr++ = (GLubyte)((tmp >> 24) & 0xff);
	    *tptr++ = (GLubyte)((tmp >> 16) & 0xff);
	    *tptr++ = (GLubyte)((tmp >> 8) & 0xff);
	    *tptr++ = (GLubyte)(tmp & 0xff);
	    offsets = (GLvoid *)tptr;
	}
	    break;
	case GL_UNSIGNED_INT:
	    SET_ARRAY(GLuint);
	    break;
	case GL_FLOAT:
	    SET_ARRAY(GLfloat);
	    break;
	case GL_3_BYTES:
	{
	    GLuint tmp;
	    GLubyte *tptr;
	    tptr = offsets;
	    tmp = dlists[i] - *base;
	    *tptr++ = (GLubyte)((tmp >> 16) & 0xff);
	    *tptr++ = (GLubyte)((tmp >> 8) & 0xff);
	    *tptr++ = (GLubyte)(tmp & 0xff);
	    offsets = (GLvoid *)tptr;
	}
	    break;
	}
    }
#if DEBUG
    ogEnvLog(2, "Display list numbers, offsets:\n");
    for(i = 0; i < n; i++) {
	ogEnvLog(2, "%d: %d %d\n", i, dlists[i], dlists[i] - *base);
    }
#endif

}

void delete_lists(GLsizei n)
{
    GLint i;
    for(i = 0; i < n; i++) {
	glDeleteLists(dlists[i], 1);
    }
}

/*
** Look for <n> rectangles of color <color>, arranged in a diagonal.
*/
static void checkRectangles(GLsizei n, 
			    unsigned int wid, 
			    unsigned int ht, 
			    unsigned int xoff, 
			    unsigned int yoff, 
			    unsigned int color)
{
  int rects;
#if DEBUG
  ogEnvLog(3, "Checking Rects:");
#endif
  for (rects = 0; rects < n; rects++) {
#if DEBUG
      ogEnvLog(3," %d, ", rects);
      if(!(rects % 5))
	  ogEnvLog(3,"\n");
#endif
      (void) ogLibRectCheck(rects*xoff + 1,
			    rects*yoff + 1,
			    rects*xoff + wid,
			    rects*yoff + ht,
			    color,
			    0);
  }
  /* should be no rectangles beyond max */
#if DEBUG
  ogEnvLog(3, "\nChecking One past last Rect\n");
#endif
  (void) ogLibRectCheck(rects*xoff + 1,
                        rects*yoff + 1,
                        rects*xoff + wid,
                        rects*yoff + ht,
                        0,
                        0);
}


/*
** So we don't have to make assumptions about the maximum size of
** legal glCallLists() types
*/
typedef union {
    GLint a;
    GLuint b;
    GLfloat c; /* don't bother with 2_BYTES, 3_BYTES, bytes, or shorts */
    GLbyte d[4]; /* for GL_4_BYTES */
} MaxSize;

MaxSize offsets[MAX_LISTS];

TESTMOD(dlist_array)
{
    GLuint xmax, ymax;
    GLuint wid, ht, color;
    GLenum type;
    GLsizei n;
    GLuint base;

    glMatrixMode(GL_PROJECTION);
    glOrtho(0.0, (double)ogEnvQuery(OG_XWSIZE), /* map EC 1 to 1 with SC */
	    0.0, (double)ogEnvQuery(OG_YWSIZE),
	    -1.,1.);

    /* generate set of dlist names & create dlists */
    xmax = ogEnvQuery(OG_XWSIZE) - 1; /* window x values 0 -> xmax */
    ymax = ogEnvQuery(OG_YWSIZE) - 1; /* window y values 0 -> ymax */
    /* restrict minimum rectangle size to 3 */
    if(xmax/6 < MaxLists)
	MaxLists = xmax/5 - 1;
    if(ymax/6 < MaxLists)
	MaxLists = ymax/5 - 1;

    wid = xmax/(MaxLists + 1) - 2;
    ht  = ymax/(MaxLists + 1) - 2;

    /* set to modelview, since this is the matrix that will be xformed */
    glMatrixMode(GL_MODELVIEW);
    type = GL_BYTE; /* rotate through types */
    while (pass--) {
	ogLibClear(0);
	glLoadIdentity();
	color = ogLibColor(); /* one color for each array of dlists */

	ogEnvLog(1, "Array type: %s\n", ogEnvDataTypeName(type));
	n = ogLibIntRand(1, MaxLists);
	ogEnvLog(1, "Number of Array elements: %d\n", n);
	make_lists(n, type, &base, offsets, wid, ht, color);
	ogEnvLog(1, "Base for Offsets: %d\n", base);
        glListBase(base);
	glCallLists(n, type, offsets);
	/* check results */
	checkRectangles(n, wid, ht, wid + 2, ht + 2, color);
	/* delete display lists */
	delete_lists(n);
	type++;
	if(type > GL_4_BYTES)
	    type = GL_BYTE;
    }
}

CLEANUP(dlist_array)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glListBase(0);
    glDeleteLists(1, GL_MAX_LIST_NESTING); /* delete created display lists */
    ogLibSetDefaultColors();
}

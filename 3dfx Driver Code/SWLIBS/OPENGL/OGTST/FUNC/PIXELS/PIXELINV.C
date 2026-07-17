/**************************************************************************
 *                                                                        *
 *               Copyright (C) 1994, Silicon Graphics, Inc.               *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 **************************************************************************/

/* pixelinv.c - $Revision: 2$ */

/*
 * test that pixels written and read using UNSIGNED formats are
 * invariant in the MSB's.  test further that pixels generated
 * by clear or geometry operations using flat shading also
 * have predicatable values when read back with UNSIGNED types.
 */

#include <ogtst.h>

#define NUM_TYPES	3
static GLenum ptype[] =  {
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_SHORT,
    GL_UNSIGNED_INT,
};

static char *tnames[3] = {
    "GL_UNSIGNED_BYTE",
    "GL_UNSIGNED_SHORT",
    "GL_UNSIGNED_INT",
};

static unsigned masks[3];
static int sshift[4][3][3];

static GLubyte ub[4];
static GLushort us[4];
static GLuint ui[4];
static char *paddr[] = { (char *)ub, (char *)us, (char *)ui };

#define min(a,b)	(a) < (b) ? (a) : (b)

void
check_pixel(unsigned *values, GLenum format, int nc, int *fbbits, GLenum dtype, unsigned *masks, unsigned *cmask) {
    GLuint fbvalue, mask, value;
    int i, c, xtype;

    /* iterate over all unsigned readback types */
    for(i = 0; i < NUM_TYPES; i++) {
	glReadPixels(1,1,1,1, format, ptype[i], paddr[i]);
	for(c = 0; c < nc; c++) {
	    switch(ptype[i]) {
	    case GL_UNSIGNED_BYTE: fbvalue = ub[c]; break;
	    case GL_UNSIGNED_SHORT: fbvalue = us[c]; break;
	    case GL_UNSIGNED_INT: fbvalue = ui[c]; break;
	    }
	    xtype = min(dtype,i);
	    mask = masks[xtype];
	    value = values[c]&cmask[c];
	    ogEnvLog(2,"comp[%d] %x %x %d %d %x\n", c, (value >> sshift[c][dtype][xtype]), (fbvalue >> sshift[c][i][xtype]), sshift[c][dtype][xtype], sshift[c][i][xtype], mask);
	    if (((value >> sshift[c][dtype][xtype])&mask) != ((fbvalue >> sshift[c][i][xtype])&mask)) {
		ogEnvLog(OG_LFAIL, "wrote comp #%d: 0x%x as type %s, read 0x%x as type %s expected 0x%x for %d-bit comp\n", c,value, tnames[dtype], fbvalue >> sshift[c][i][xtype], tnames[i], value >> sshift[c][dtype][xtype], fbbits[c]);
	    }
	}
    }
}

TESTMOD(pixelinv)
{
    GLboolean rgbMode = ogEnvCurVisualInfo(GLX_RGBA);
    int fbbits[4];
    int i;
    int xsize = ogEnvQuery(OG_XWSIZE);
    int ysize = ogEnvQuery(OG_YWSIZE);
    unsigned value[4];
    unsigned cmask[4];

    glMatrixMode(GL_PROJECTION);
    glOrtho(0, xsize, 0, ysize, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_DITHER);
    glShadeModel(GL_FLAT);

    if (rgbMode) {
	int nc;

	fbbits[0] = ogEnvCurVisualInfo(GLX_RED_SIZE);
	fbbits[1] = ogEnvCurVisualInfo(GLX_GREEN_SIZE);
	fbbits[2] = ogEnvCurVisualInfo(GLX_BLUE_SIZE);
	fbbits[3] = ogEnvCurVisualInfo(GLX_ALPHA_SIZE);

	masks[0] = masks[1] = masks[2] = 0xffffffff;

	nc = fbbits[3] > 0 ? 4 : 3;

	for(i = 0; i < 4; i++) {
	    int msbs_ub = min(8, fbbits[i]);
	    int msbs_us = min(16, fbbits[i]);
	    int msbs_ui = min(32, fbbits[i]);

	    sshift[i][0][0] = msbs_ub < 8 ? 8 - msbs_ub : 0;
	    sshift[i][0][1] = msbs_ub < 8 ? 8 - msbs_ub : 0;
	    sshift[i][0][2] = msbs_ub < 8 ? 8 - msbs_ub : 0;
	    sshift[i][1][0] = msbs_ub < 16 ? 16 - msbs_ub : 0;
	    sshift[i][1][1] = msbs_us < 16 ? 16 - msbs_us : 0;
	    sshift[i][1][2] = msbs_us < 16 ? 16 - msbs_us : 0;
	    sshift[i][2][0] = msbs_ub < 32 ? 32 - msbs_ub : 0;
	    sshift[i][2][1] = msbs_us < 32 ? 32 - msbs_us : 0;
	    sshift[i][2][2] = msbs_ui < 32 ? 32 - msbs_ui : 0;
	    cmask[i] = fbbits[i] ? 0xffffffff : 0;
	}

	/* draw pixels */
	glRasterPos2f(1., 1.);

	while(pass--) {
	    for(i = 0; i < NUM_TYPES; i++) {
		value[0] = (ogLibBitRand(31) << 1);
		value[1] = (ogLibBitRand(31) << 1);
		value[2] = (ogLibBitRand(31) << 1);
		value[3] = (ogLibBitRand(31) << 1);
		ogEnvLog(1, "value (0x%x,0x%x,0x%x,0x%x)\n", value[0],value[1],value[2],value[3]);
		switch(ptype[i]) {
		case GL_UNSIGNED_BYTE:  ub[0] = value[0] >>= 24; ub[1] = value[1] >>= 24;
					ub[2] = value[2] >>= 24; ub[3] = value[3] >>= 24; break;
		case GL_UNSIGNED_SHORT: us[0] = value[0] >>= 16; us[1] = value[1] >>= 16;
					us[2] = value[2] >>= 16; us[3] = value[3] >>= 16; break;
		case GL_UNSIGNED_INT:   ui[0] = value[0]; ui[1] = value[1]; ui[2] = value[2]; ui[3] = value[3]; break;
		}
		glDrawPixels(1, 1, GL_RGBA, ptype[i], paddr[i]);
		check_pixel(value, GL_RGBA, nc, fbbits, i, masks, cmask);
	    }

	    /* clear + clear color */

#if 1
	    /* geometry + color */
	    for(i = 0; i < 3; i++) {
		value[0] = (ogLibBitRand(31) << 1);
		value[1] = (ogLibBitRand(31) << 1);
		value[2] = (ogLibBitRand(31) << 1);
		value[3] = (ogLibBitRand(31) << 1);
		ogEnvLog(1, "value (0x%x,0x%x,0x%x,0x%x)\n", value[0],value[1],value[2],value[3]);
		switch(i) {
		case 0:	value[0] >>= 24; value[1] >>= 24; value[2] >>= 24; value[3] >>= 24;
			glColor4ub(value[0], value[1], value[2], value[3]); break;
		case 1:	value[0] >>= 16; value[1] >>= 16; value[2] >>= 16; value[3] >>= 16;
			glColor4us(value[0], value[1], value[2], value[3]); break;
		case 2: glColor4ui(value[0], value[1], value[2], value[3]); break;
		}
		glRecti(0,0,4,4);
		check_pixel(value, GL_RGBA, nc, fbbits, i, masks, cmask);
	    }
#endif
	}
    } else {
	fbbits[0] = ogEnvCurVisualInfo(GLX_BUFFER_SIZE);
	sshift[0][0][0] = 0;	/* color index is always lsb justified */
	sshift[0][0][1] = 0;
	sshift[0][0][2] = 0;
	sshift[0][1][0] = 0;
	sshift[0][1][1] = 0;
	sshift[0][1][2] = 0;
	sshift[0][2][0] = 0;
	sshift[0][2][1] = 0;
	sshift[0][2][2] = 0;
	masks[0] = ((1 << fbbits[0])-1)&0xff;
	masks[1] = ((1 << fbbits[0])-1)&0xffff;
	masks[2] = ((1 << fbbits[0])-1)&0xffffffff;
	cmask[0] = 0xffffffff;
	

	/* draw pixels */
	glRasterPos2f(1., 1.);

	while(pass--) {
	    for(i = 0; i < NUM_TYPES; i++) {
		value[0] = ogLibBitRand(31) << 1;
		ogEnvLog(1, "value 0x%x\n", value[0]);
		switch(ptype[i]) {
		case GL_UNSIGNED_BYTE:  ub[0] = value[0] >>= (32-8); break;
		case GL_UNSIGNED_SHORT: us[0] = value[0] >>= (32-16); break;
		case GL_UNSIGNED_INT:   ui[0] = value[0]; break;
		}
		glDrawPixels(1, 1, GL_COLOR_INDEX, ptype[i], paddr[i]);
		check_pixel(value, GL_COLOR_INDEX, 1, fbbits, i, masks, cmask);
	    }

	    /* clear + clear index */


#if 0
	    /* geometry + index */
	    for(i = 1; i < 3; i++) {
		value[0] = ogLibBitRand(31) << 1;
		ogEnvLog(1, "value 0x%x\n", value[0]);
		switch(i) {
		/*case 0:	value[0] >>= 24; glIndexb(value[0]); break;*/
		case 1:	value[0] >>= 16; value[0] &= 0x7fff; glIndexs(value[0]); break;
		case 2: 	         value[0] &= 0x7fffffff; glIndexi(value[0]);
					 value[0] = (float)value[0];	/* float conversions truncates to 24 bits */
					 break;
		}
		/*{ GLint v; glGetIntegerv(GL_CURRENT_INDEX, &v); ogEnvLog(OG_LALWAYS,"v = 0x%x\n", v); }*/
		glRecti(0,0,4,4);
		check_pixel(value, GL_COLOR_INDEX, 1, fbbits, i, masks, cmask);
	    }
#endif
	}
    }
}

CLEANUP(pixelinv)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DITHER);
    glShadeModel(GL_SMOOTH);
    ogLibSetDefaultColors();
    ogLibSetDefaultRasterPos();
}

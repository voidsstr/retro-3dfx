/*
** Utility functions for texture tests.
*/
#include <bstring.h>
#include "ogtst.h"
#include "utilfont.h"

unsigned char chartricolors[][4] = {
    { 0xff, 0xff, 0x00, 0xff },
    { 0x00, 0x00, 0xff, 0xff },
    { 0x80, 0x80, 0x80, 0xff },
};
unsigned short shorttricolors[][4] = {
    { 0xff00, 0xff00, 0x0000, 0xffff },
    { 0x0000, 0x0000, 0xff00, 0xffff },
    { 0x8000, 0x8000, 0x8000, 0xffff },
};
unsigned int inttricolors[][4] = {
    { 0xff000000, 0xff000000, 0x00000000, 0xffffffff },
    { 0x00000000, 0x00000000, 0xff000000, 0xffffffff },
    { 0x80000000, 0x80000000, 0x80000000, 0xffffffff },
};

static void
fillpix(unsigned char *p, unsigned char color[4], GLenum texfmt,
	GLenum textype)	
{
    int cmpcnt, cmpsize, texelsize;
    int i;

    cmpcnt = ogLibCompsPerPixel(texfmt);
    cmpsize = ogLibBytesPerComp(textype);
    texelsize = cmpcnt * cmpsize;

    if (cmpsize == 1) {
	bcopy(color, p, texelsize);
	return;
    }
    for (i=0; i < cmpcnt; i++) {
	switch (textype) {
	  case GL_UNSIGNED_SHORT:
	    *(unsigned short *)p = color[i] << 8;
	    break;
	  case GL_UNSIGNED_INT:
	    *(unsigned int   *)p = color[i] << 16;
	    break;
	}
	p += cmpsize;
    }
}

static void
gentri(int ssize, int tsize, GLenum texfmt, GLenum textype, unsigned char *fg,
       unsigned char *bg, char *ip)
{
    int t, i;
    int texsize, bglen, spanlen;
    float fbglen, dsdt, c, dcdt;
    unsigned char *p, *color;
    int cmpcnt = ogLibCompsPerPixel(texfmt);
    int cmpsize = ogLibBytesPerComp(textype);

    c = 0;
    dsdt = (ssize/2.0) / tsize;
    dcdt = 1.0 / tsize;

    texsize = cmpcnt * cmpsize;

    bglen = 0;
    fbglen = 0;
    spanlen = ssize / 2 - bglen;
    for (t=0; t < tsize; t++) {
	p = (unsigned char *)(ip + t * ssize * texsize);
	for (i=0; i < bglen; i++) {
	    color = bg ? bg : chartricolors[2];
	    fillpix(p, color, texfmt, textype);
	    p += texsize;
	}
	for (i=0; i < spanlen; i++) {
	    color = fg ? fg : chartricolors[0];
	    fillpix(p, color, texfmt, textype);
	    p += texsize;
	}
	for (i=0; i < spanlen; i++) {
	    color = fg ? fg : chartricolors[1];
	    fillpix(p, color, texfmt, textype);
	    p += texsize;
	}
	for (i=0; i < bglen; i++) {
	    color = bg ? bg : chartricolors[2];
	    fillpix(p, color, texfmt, textype);
	    p += texsize;
	}
	c += dcdt;
	fbglen += dsdt;
	bglen = (int) fbglen;
	spanlen = ssize / 2 - bglen;
    }
}

void *
ogLibGenTriTex(int ssize, int tsize, GLenum texfmt, GLenum textype,
               unsigned char *fg, unsigned char *bg)
{
    int sz;
    char *p;

    sz = ogLibImageSize(ssize, tsize, texfmt, textype);
    p = (char *) ogLibMalloc(sz); 
    gentri(ssize, tsize, texfmt, textype, fg, bg, p);
    return p;
}

void *
ogLibGetTriTex3d(int ssize, int tsize, int rsize, GLenum texfmt, GLenum textype)
{
    int r, sz;
    char *p;

    sz = ogLibImageSize(ssize, tsize, texfmt, textype);
    p = (char *) ogLibMalloc(sz*rsize); 

    for (r=0; r < rsize; r++) {
	gentri(ssize, tsize, texfmt, textype, 0, 0, p + r*sz);
    }
    return p;
}

void *
ogLibGenSolidTex(int ssize, int tsize, GLenum texfmt, GLenum textype,
                 unsigned char *col)
{
    int i, t, sz, texsize;
    char *p, *ip;
    int cmpcnt = ogLibCompsPerPixel(texfmt);
    int cmpsize = ogLibBytesPerComp(textype);

    sz = ogLibImageSize(ssize, tsize, texfmt, textype);
    ip = (char *) ogLibMalloc(sz); 

    texsize = cmpcnt * cmpsize;
    for (t=0; t < tsize; t++) {
	p = ip + t * ssize * texsize;
	for (i=0; i < ssize; i++) {
	    fillpix((unsigned char *)p, col, texfmt, textype);
	    p += texsize;
	}
    }
    return ip;
}

void *
ogLibGenFrameTex(int ssize, int tsize, GLenum texfmt, GLenum textype,
                 unsigned char *fg, unsigned char *bg)
{
    int s, t, sz, texsize;
    unsigned char *p, *ip;
    int cmpcnt = ogLibCompsPerPixel(texfmt);
    int cmpsize = ogLibBytesPerComp(textype);
    float sf, tf, sd, td;

    sz = ogLibImageSize(ssize, tsize, texfmt, textype);
    ip = (unsigned char *) ogLibMalloc(sz); 

    sd = ssize / 2 + 0.5;
    td = tsize / 2 + 0.5;

    texsize = cmpcnt * cmpsize;
    for (t=0; t < tsize; t++) {
	p = ip + t * ssize * texsize;
	for (s=0; s < ssize; s++) {
	    sf = fabs((s - sd) / sd);
	    tf = fabs((t - td) / td);
	    if (!(sf <= 0.75 && tf <= 0.75) && !(sf > 1.0 || tf > 1.0)) {
		fillpix(p, fg, texfmt, textype);
	    } else {
		fillpix(p, bg, texfmt, textype);
	    }
	    p += texsize;
	}
    }
    return ip;
}

void *
ogLibGenLabelTex(int ssize, int tsize, GLenum texfmt, GLenum textype,
                 unsigned char *fg, unsigned char *bg, char *label)
{
    int lw, lh;
    int s, t, ss, ts, sp, tp;
    int i, j, m, n;
    int mjmp, njmp;
    int c, sz, slen;
    int mag, reduce=0, redsize;
    unsigned char *p, *tex;
    unsigned char *gl, src;
    unsigned char blend[4];
    int texsize;
    unsigned int sum;
    float scale;

    slen = strlen(label);
    if (ssize >= slen*FONTIMGSIZE) {
	mag = ssize / (slen*FONTIMGSIZE);
	for (i=0; mag; i++) mag >>= 1;
	scale = 1 << (i-1);
    } else {
	reduce = 2;
	while (!(ssize*reduce / (slen*FONTIMGSIZE))) {
	    reduce <<= 1;
	}
	scale = 1.0 / reduce;
    }
    
    /* width and height of label */
    lw = slen * FONTIMGSIZE * scale;
    lh = FONTIMGSIZE * scale;
    /* position of label */
    ss = (ssize - lw) / 2;
    ts = (tsize - lh) / 2;

    texsize = ogLibCompsPerPixel(texfmt) * ogLibBytesPerComp(textype);
    sz = ssize * tsize * texsize;
    p = (unsigned char *) ogLibMalloc(sz); 

    /* fill all texture with background */
    tex = p;
    for (t=0; t < tsize; t++) {
	for (s=0; s < ssize; s++) {
	    fillpix(tex, bg, texfmt, textype);
	    tex += texsize;
	}
    }
    /* write label on top */
    for (c=0; c < slen; c++) {
	gl = glyphs[label[c]-'0'];
	if (label[c] == ' ') {
	    gl = glyphs['9'-'0' + 1];
	}

	if (scale < 1.0) {
	    redsize = FONTIMGSIZE/reduce;
	    for (m=0; m < redsize; m++) {
		mjmp = m*reduce;
		for (n=0; n < redsize; n++) {
		    njmp = n*reduce;
		    sum = 0;
		    for (i=0; i < reduce; i++) {
			for (j=0; j < reduce; j++) {
			    sum += gl[((mjmp + i) * FONTIMGSIZE) +
				     (njmp + j)];
			}
		    }
		    sum = (sum / (reduce*reduce));

		    sp = ss + c*redsize+n;
		    tp = ts + m;
		    tex = p + (tp * ssize + sp) * texsize;
		    blend[0] = (fg[0] * sum + tex[0] * (0xff-sum)) / 0xff;
		    blend[1] = (fg[1] * sum + tex[1] * (0xff-sum)) / 0xff;
		    blend[2] = (fg[2] * sum + tex[2] * (0xff-sum)) / 0xff;
		    blend[3] = (fg[3] * sum + tex[3] * (0xff-sum)) / 0xff;
		    fillpix(tex, blend, texfmt, textype);
		}
	    }
	} else {
	    for (m=0; m < FONTIMGSIZE; m++) {
		for (n=0; n < FONTIMGSIZE; n++) {
		    src = gl[m*FONTIMGSIZE + n];
		    if (src == 0xff) {
			/* i and j are for scaling in s and t directions */
			for (i=0; i < scale; i++) {
			    for (j=0; j < scale; j++) {
				sp = ss + (c*FONTIMGSIZE + n)*scale + j;
				tp = ts + m*scale + i;
				tex = p + (tp * ssize + sp) * texsize;
				fillpix(tex, fg, texfmt, textype);
			    }
			}
		    }
		}
	    }
	}
    }
    return p;
}

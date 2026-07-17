#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include <windows.h>
#include <glide.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "winutil.h"
#include "clock.h"

float period = 2.0;
int numtex = 10;
enum { MIP, NOMIP, ALTMIP };
int mipmode = MIP;
char *mipstr = "mip";

unsigned int blue = 0x00ff0000;
unsigned int yellow = 0x0000ffff;

static void
init_tex(void) {
    int texsize = 256;
    unsigned int *tex = malloc(texsize * texsize * sizeof(unsigned int));
    unsigned int *p = tex;
    int i, j;

    for (j=0; j < texsize; j++) {
	for (i=0; i < texsize; i++) {
	    if (((i/8) + (j/8)) % 2) {
		*p++ = blue;
	    } else {
		*p++ = yellow;
	    }
	}
    }
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, texsize, texsize,
		      GL_RGBA, GL_UNSIGNED_BYTE, tex);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    free(tex);
}

GLuint *tnames;

static void
init(void) {
    int i;

    glMatrixMode(GL_PROJECTION);
    glOrtho(0, 640, 0, 480, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1, 0, 0, 1);
    glShadeModel(GL_FLAT);

    tnames = (GLuint *) malloc(sizeof(GLuint) * numtex);
    glGenTextures(numtex, tnames);
    for (i=0; i < numtex; i++) {
	glBindTexture(GL_TEXTURE_2D, tnames[i]);
	init_tex();
    }
}

static void
do_bind(int iter) {
    int i, j;

    for (j=0; j < iter; j++) {
	for (i=0; i < numtex; i++) {
	    glBindTexture(GL_TEXTURE_2D, tnames[i]);
	}
    }
}

static void
run_bench(void) {
    int iter = 100;
    int numbinds;
    float elapsed = 0;
    float bindspersec;

    /* find a number of reps to match the desired period */
    while (elapsed < period) {
	iter *= 2;
	startclock();
	do_bind(iter);
	glFinish();
	elapsed = stopclock();
    }

    startclock();
    do_bind(iter);
    glFinish();
    elapsed = stopclock();

    numbinds = numtex * iter;
    bindspersec = numbinds / elapsed;
    printf("%-8s elapsed: %4.2f binds: %6d binds/sec: %6.0f\n",
	   mipstr, elapsed, numbinds, bindspersec);
}

void usage(void) {
    printf("usage: trispeed [-n numbinds][-p period][-m mipode\n");
    exit(1);
}

void main( int argc, char **argv) {
    int i, n;

    for (i=1; i < argc; i++) {
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {
	    case 'n':
		if (++i >= argc) usage();
		numtex = atoi(argv[i]);
		break;
	    case 'p':
		if (++i >= argc) usage();
		period = atof(argv[i]);
		break;
	    case 'm':
		if (++i >= argc) usage();
		if (!strcmp(argv[i], "nomip")) {
		    mipmode = NOMIP;
		} else if (!strcmp(argv[i], "mip")) {
		    mipmode = MIP;
		} else if (!strcmp(argv[i], "altmip")) {
		    mipmode = ALTMIP;
		} else {
		    usage();
		}
		break;
	    default:
		usage();
	    }
	}
    }
    
    initApplication(GetModuleHandle(NULL), 0, FALSE);

    init();
    glClear(GL_COLOR_BUFFER_BIT);
    run_bench();
    CHKERROR();
}


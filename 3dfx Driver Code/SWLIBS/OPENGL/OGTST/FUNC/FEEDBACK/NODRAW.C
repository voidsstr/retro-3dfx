#include <ogtst.h>

static void draw_stuff(void);
static void check_window(void);

static int xmax, ymax;
static GLubyte bitmap[512];	/* 64x64 bitmap */
static GLubyte image[4*100];	/* 10x10 rgba image */

/*ARGSUSED*/
TESTMOD(nodraw)
{
    int i;
    static GLuint select;
    static GLfloat feedback;

    glClearColor(0.5, 0.5, 0.5, 0.5);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glOrtho(-1.4,1.4,-1.4,1.4,1.4,-1.4);

    for (i = 0; i < 512; i++) {
	bitmap[i] = ogLibBitRand(8);
    }

    for (i = 0; i < 400; i++) {
	image[i] = ogLibBitRand(8);
    }
    
    for (i = 0; i < 2; i++) {
	if (i == 0) {
	    ogEnvLog(1,"glRenderMode(GL_SELECT);\n");
	    glSelectBuffer(1, &select);
	    glRenderMode(GL_SELECT);
	} else {
	    ogEnvLog(1,"glRenderMode(GL_FEEDBACK);\n");
	    glFeedbackBuffer(1, GL_2D, &feedback);
	    glRenderMode(GL_FEEDBACK);
	}

	draw_stuff();

	check_window();

	(void) glRenderMode(GL_RENDER);
    }
}

CLEANUP(nodraw)
{
    (void) glRenderMode(GL_RENDER);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ogLibSetDefaultRasterPos();
    glClearColor(0, 0, 0, 0);
    glClearAccum(0., 0., 0., 0.);
    glSelectBuffer(0, 0);
    glFeedbackBuffer(0, GL_2D, 0);
}

static void check_window(void) {
    int i, num_pixels;
    GLuint *pixels;
    
    xmax = ogEnvQuery(OG_XWSIZE);
    ymax = ogEnvQuery(OG_YWSIZE);
    num_pixels = xmax * ymax;
    
    pixels = (GLuint *) ogLibMalloc(num_pixels*sizeof(GLuint));
    glReadPixels(0, 0, xmax, ymax, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    for (i = 0; i < num_pixels; i++) {
	if (ogLibColCheck(pixels[i], 0x7f7f7f7f, 0)) {
	    ogEnvLog(OG_LFAIL, "(%d, %d) got: %#x, expected: %#x\n",
		     i%xmax, i/xmax, pixels[i], 0x7f7f7f7f);
	    
	}
    }
    ogLibFree(pixels);
}

static void draw_stuff(void)
{
    float x,y,z,w;
    int i,limit;
    int accum_bits;
    
    glGetIntegerv(GL_ACCUM_RED_BITS, &accum_bits);
    limit = ogLibBitRand(4);

    ogEnvLog(2,"\tTest: glClear\n");
    glClearColor(.1, .1, .1, .1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    ogEnvLog(2,"\tTest: GL_TRIANGLE_STRIP\n");
    glBegin(GL_TRIANGLE_STRIP);
    for (i=0; i<3+limit; i++) {
	x = ogLibFloatRand(-1.0,1.0);
	y = ogLibFloatRand(-1.0,1.0);
	z = ogLibFloatRand(-1.0,1.0);
	w = ogLibFloatRand( 0.9,1.1);
	glVertex4f(x,y,z,w);
    }
    glEnd();
    
    ogEnvLog(2,"\tTest: GL_LINE_STRIP\n");
    glBegin(GL_LINE_STRIP);
    for (i=0; i<2+limit; i++) {
	x = ogLibFloatRand(-1.0,1.0);
	y = ogLibFloatRand(-1.0,1.0);
	z = ogLibFloatRand(-1.0,1.0);
	w = ogLibFloatRand( 0.9,1.1);
	glVertex4f(x,y,z,w);
    }
    glEnd();
    
    ogEnvLog(2,"\tTest: GL_POINTS\n");
    glBegin(GL_POINTS);
    for (i=0; i<1+limit; i++) {
	x = ogLibFloatRand(-1.0,1.0);
	y = ogLibFloatRand(-1.0,1.0);
	z = ogLibFloatRand(-1.0,1.0);
	w = ogLibFloatRand( 0.9,1.1);
	glVertex4f(x,y,z,w);
    }
    glEnd();
    
    ogEnvLog(2,"\tTest: glBitmap\n");
    x = ogLibFloatRand(-1.0,1.0);
    y = ogLibFloatRand(-1.0,1.0);
    z = ogLibFloatRand(-1.0,1.0);
    w = ogLibFloatRand( 0.9,1.1);
    glRasterPos4f(x,y,z,w);
    glBitmap(64, 64, 5, 7, 128, 0, bitmap);
    
    ogEnvLog(2,"\tTest: glDrawPixels\n");
    x = ogLibFloatRand(-1.0,1.0);
    y = ogLibFloatRand(-1.0,1.0);
    z = ogLibFloatRand(-1.0,1.0);
    w = ogLibFloatRand( 0.9,1.1);
    glRasterPos4f(x,y,z,w);
    glDrawPixels(10, 10, GL_RGBA, GL_UNSIGNED_BYTE, image);

    ogEnvLog(2,"\tTest: glCopyPixels\n");
    x = ogLibFloatRand(-1.0,1.0);
    y = ogLibFloatRand(-1.0,1.0);
    z = ogLibFloatRand(-1.0,1.0);
    w = ogLibFloatRand( 0.9,1.1);
    glRasterPos4f(x,y,z,w);
    x = ogLibIntRand(10, xmax - 10);
    y = ogLibIntRand(10, ymax - 10);
    glCopyPixels(0, 0, xmax, ymax, GL_COLOR);

    if (accum_bits) {
	ogEnvLog(2,"\tTest: glAccum\n");
	glClearAccum(.25, .25, .25, .25);
	glClear(GL_ACCUM_BUFFER_BIT);
	glAccum(GL_RETURN, 0.5);
    }
}

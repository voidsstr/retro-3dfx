#include <stdio.h>
#include "ogtst.h"

#define CHECK_RESULT(p, e)						 \
{									 \
    GLfloat f;								 \
    glGetFloatv((p), &f);						 \
    if (f != (e)) {							 \
        ogEnvLog(OG_LFAIL, "Got: %f %#x, expected: %f %#x\n", f, f, (e), \
		 (e));  						 \
    }									 \
}

#define CHECK_RESULTS2(p, e0, e1)					\
{									\
    GLfloat f[2], e[2] = {e0, e1};					\
    int i;								\
    glGetFloatv((p), f);						\
    for (i = 0; i < 2; i++) {						\
	if (f[i] != e[i]) {						\
	    ogEnvLog(OG_LFAIL, "[%d] Got: %f %#x, expected: %f %#x\n",	\
		     i, f[i], f[i], e[i], e[i]);			\
	}								\
    }									\
}

#define CHECK_RESULTS4(p, e0, e1, e2, e3)				   \
{									   \
    GLfloat f[4], e[4] = {e0, e1, e2, e3};				   \
    int i;								   \
    glGetFloatv((p), f);						   \
    for (i = 0; i < 4; i++) {						   \
	if (f[i] != e[i]) {						   \
	    ogEnvLog(OG_LFAIL, "[%d] Got: %f (%#x), expected: %f (%#x)\n", \
		     i, f[i], f[i], e[i], e[i]);			   \
	}								   \
    }									   \
}

#define CHECK_DEPTH(x, y, v)						      \
{									      \
     GLfloat f, tolerance = 0.0001;					      \
     glReadPixels(11, 11, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, (GLvoid *)&f);  \
     if (fabs(f-(v)) > tolerance) {					      \
	 ogEnvLog(OG_LFAIL,						      \
		  "(%d, %d) depth check Got: %f (%#x), expected: %f (%#x)\n", \
		  (x), (y), f, f, (v), (v));				      \
     }									      \
}

#define TEST(t) \
    ogEnvLog(1, #t"\n"); \
    t

#define HAVE_EXT(ext) (strstr((const char *)ex, ext) != 0)

static int xmax, ymax;

void draw_line(void) {
    glBegin(GL_LINES);
       glVertex2i(10, ymax/2);
       glVertex2i(xmax - 10, ymax/2);
    glEnd();
    glFlush();
}

void draw_point(void) {
    glBegin(GL_POINTS);
       glVertex2i(xmax/2, ymax/2);
    glEnd();
    glFlush();
}

/*ARGSUSED*/
TESTMOD(clamp)  {
    const GLubyte *ex = glGetString(GL_EXTENSIONS);
    xmax = ogEnvQuery(OG_XWSIZE);
    ymax = ogEnvQuery(OG_YWSIZE);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0., xmax, 0., ymax, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    
    /* Alpha Func */
    glEnable(GL_ALPHA_TEST);
    TEST(glAlphaFunc(GL_EQUAL, 10.0));
    CHECK_RESULT(GL_ALPHA_TEST_REF, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor4ub(1, 2, 3, 0xff);
    glRecti(10, 10, 12, 12);
    ogLibPixelCheck(11, 11, 0x010203ff);

    TEST(glAlphaFunc(GL_EQUAL, -10.0));    
    CHECK_RESULT(GL_ALPHA_TEST_REF, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor4ub(1, 2, 3, 0x00);
    glRecti(10, 10, 12, 12);
    ogLibPixelCheck(11, 11, 0x01020300);
    glDisable(GL_ALPHA_TEST);
    glAlphaFunc(GL_ALWAYS, 0.0);
    
    /* Clear Color */
    TEST(glClearColor(10.0, 10.0, 10.0, 10.0));
    CHECK_RESULTS4(GL_COLOR_CLEAR_VALUE, 1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ogLibPixelCheck(11, 11, 0xffffffff);

    TEST(glClearColor(-10.0, -10.0, -10.0, -10.0));
    CHECK_RESULTS4(GL_COLOR_CLEAR_VALUE, 0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ogLibPixelCheck(11, 11, 0);

    /* Clear Depth */
    TEST(glClearDepth(-10.0));
    CHECK_RESULT(GL_DEPTH_CLEAR_VALUE, 0.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    CHECK_DEPTH(11, 11, 0.0);

    TEST(glClearDepth(10.0));
    CHECK_RESULT(GL_DEPTH_CLEAR_VALUE, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    CHECK_DEPTH(11, 11, 1.0);

    /* Depth Range */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glClear(GL_DEPTH_BUFFER_BIT);
    TEST(glDepthRange(10.0, 10.0));
    CHECK_RESULTS2(GL_DEPTH_RANGE, 1.0, 1.0);
    glBegin(GL_TRIANGLES);
        glVertex3f(5, 5, 0.5);
        glVertex3f(100, 5, 0.5);
        glVertex3f(5, 100, 0.5);
    glEnd();
    CHECK_DEPTH(11, 11, 1.0);

    glClear(GL_DEPTH_BUFFER_BIT);
    TEST(glDepthRange(-10.0, -10.0));
    CHECK_RESULTS2(GL_DEPTH_RANGE, 0.0, 0.0);
    glBegin(GL_TRIANGLES);
        glVertex3f(5, 5, 0.5);
        glVertex3f(100, 5, 0.5);
        glVertex3f(5, 100, 0.5);
    glEnd();
    CHECK_DEPTH(11, 11, 0.0);

    glDepthRange(0, 1);
    glDepthFunc(GL_LESS);
    glDisable(GL_DEPTH_TEST);
}

CLEANUP(clamp) {
    ogLibSetDefaultColors();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLineWidth(1);
    glPointSize(1);
}

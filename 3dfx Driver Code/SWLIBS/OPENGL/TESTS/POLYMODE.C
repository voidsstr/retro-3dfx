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

#pragma warning(disable : 4244)

#define CHKERROR() \
{ \
    int e = glGetError(); \
    if (e) printf("got error %d at line %d\n", e, __LINE__); \
}

float winw = 640.0f;
float winh = 480.0f;
float rectw, recth;
float spacew, spaceh;

float red[] = { 1, 0, 0, 1 };
float yellow[] = { 1, 1, 0, 1 };
float blue[] = { 0, 0, 1, 1 };

float lightpos[4] = { 0, 0, 1000, 1 };
float black[4] = { 0, 0, 0, 1 };
float white[4] = { 1, 1, 1, 1 };
float gray[4] = { .5, .5, .5, 1 };

static void
init(void) {
    glMatrixMode(GL_PROJECTION);
    glOrtho(0,winw,0,winh,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0,0,0,1);

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, black);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, white);
    glMaterialfv(GL_BACK, GL_DIFFUSE, red);
    glMaterialfv(GL_FRONT, GL_AMBIENT, black);
    glMaterialfv(GL_BACK, GL_AMBIENT, black);

    glPointSize(4.5);

    spacew = winw / 4.0f * 0.335f; 
    spaceh = winh / 3.0f * 0.335f;
    rectw = 0.9f * spacew;
    recth = 0.9f * spaceh;
}

float true[] = { 1.0 };
float false[] = { 0.0 };

static void
two_sided(void) {
    glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, true);
}

static void
one_sided(void) {
    glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, false);
}

static void
draw_tri_cw(void) {
    float xlen, ylen;
    float fx = 0.30, fy = 0.15;
    
    xlen = rectw;
    ylen = recth;
    glNormal3f(0, 0, -1);

    glBegin(GL_TRIANGLE_STRIP);

    glColor3fv(yellow);
    glMaterialfv(GL_BACK, GL_DIFFUSE, yellow);
    glVertex3f(0,ylen,0); 

    glColor3fv(red);
    glMaterialfv(GL_BACK, GL_DIFFUSE, red);
    glVertex3f(xlen*fx,ylen*fy,0); 

    glColor3fv(red);
    glMaterialfv(GL_BACK, GL_DIFFUSE, red);
    glVertex3f(0,0,0); 

    glColor3fv(yellow);
    glMaterialfv(GL_BACK, GL_DIFFUSE, yellow);
    glVertex3f(xlen,0,0);
    
    glEnd();
}

static void
draw_tri_ccw(void) {
    float xlen, ylen;
    float fx = 0.30, fy = 0.15;
    
    xlen = rectw;
    ylen = recth;
    glNormal3f(0, 0, 1);

    glBegin(GL_TRIANGLE_STRIP);
    
    glColor3fv(yellow);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow);
    glVertex3f(0,ylen,0); 

    glColor3fv(red);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
    glVertex3f(0,0,0); 

    glColor3fv(red);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
    glVertex3f(xlen*fx,ylen*fy,0); 

    glColor3fv(yellow);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow);
    glVertex3f(xlen,0,0); 

    glEnd();
}

static void
antialias(void) {
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
}

static void
alias(void) {
    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POINT_SMOOTH);
}

static void
smooth(void) {
    glShadeModel(GL_SMOOTH);
}

static void
flat(void) {
    glShadeModel(GL_FLAT);
}

static void
draw_scene(void) {

    glClear(GL_COLOR_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();

    /* row 1 */
    alias();
    flat();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);

    antialias();
    flat();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);
    
    /* row 2 */
    glLoadIdentity();
    glTranslatef(0, spaceh, 0);
    glEnable(GL_LIGHTING);
    one_sided();

    alias();
    flat();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);

    antialias();
    flat();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);

    /* row 3 */
    glLoadIdentity();
    glTranslatef(0, 2*spaceh, 0);
    two_sided();

    alias();
    flat();
    draw_tri_cw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_cw();
    glTranslatef(spacew, 0, 0);

    antialias();
    flat();
    draw_tri_cw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_cw();
    glTranslatef(spacew, 0, 0);

    /* row 1 column 2 */
    glLoadIdentity();
    glTranslatef(4*spacew, 0, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_LIGHTING);

    alias();
    flat();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);

    antialias();
    flat();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);

    /* row 2 column 2 */
    glLoadIdentity();
    glTranslatef(4*spacew, spaceh, 0);
    glEnable(GL_LIGHTING);
    one_sided();

    alias();
    flat();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);

    antialias();
    flat();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);

    /* row 3 columne 2 */
    glLoadIdentity();
    glTranslatef(4*spacew, 2*spaceh, 0);
    two_sided();

    alias();
    flat();
    draw_tri_cw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_cw();
    glTranslatef(spacew, 0, 0);

    antialias();
    flat();
    draw_tri_cw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_cw();
    glTranslatef(spacew, 0, 0);

    /* row 1 column 3 */
    glLoadIdentity();
    glTranslatef(8*spacew, 0, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    glDisable(GL_LIGHTING);

    alias();
    flat();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);

    antialias();
    flat();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);

    /* row 2 column 3 */
    glLoadIdentity();
    glTranslatef(8*spacew, spaceh, 0);
    glEnable(GL_LIGHTING);
    one_sided();

    alias();
    flat();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);

    antialias();
    flat();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_ccw();
    glTranslatef(spacew, 0, 0);

    /* row 3 columne 3 */
    glLoadIdentity();
    glTranslatef(8*spacew, 2*spaceh, 0);
    two_sided();

    alias();
    flat();
    draw_tri_cw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_cw();
    glTranslatef(spacew, 0, 0);

    antialias();
    flat();
    draw_tri_cw();
    glTranslatef(spacew, 0, 0);
    smooth();
    draw_tri_cw();
    glTranslatef(spacew, 0, 0);

    CHKERROR();
}

void main( int argc, char **argv) {
    initApplication(GetModuleHandle(NULL), 0, FALSE);

    init();
    draw_scene();

    while (!getkey());
    return;
}

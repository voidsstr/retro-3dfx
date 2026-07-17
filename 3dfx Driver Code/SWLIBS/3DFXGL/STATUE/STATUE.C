#define PROGRAM "statue"
#define VERSION "3.00"
/*
 *  statue.c
 *                                                     08/10/95  1.00  NDR
 *                                                     12/10/95  2.00  NDR
 *                                                     02/05/96  3.00  NDR
 *
 *    Copyright (C) 1994 by Evans & Sutherland
 *    author: Nate Robins
 *    email : narobins@es.com
 *
 *
 *    Copyright 1996 by Evans & Sutherland Computer Corporation.  
 *    ALL RIGHTS RESERVED. 
 *    
 *    Permission to use, copy, modify, and distribute this software 
 *    for any purpose and without fee is hereby granted, provided 
 *    that the above copyright notice appear in all copies and that 
 *    both the copyright notice and this notice appear in supporting
 *    documentation.  You may not use the name "Evans & Sutherland 
 *    Computer Corporation" or any trademark of Evans & Sutherland 
 *    Computer Corporation in advertising or publicity pertaining 
 *    to the software without specific, written prior permission. 
 *    
 *    THIS SOFTWARE AND ANY ASSOCIATED MATERIALS IS PROVIDED TO YOU 
 *    "AS-IS" AND WITHOUT REPRESENTATION OR WARRANTY OF ANY KIND, 
 *    EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, 
 *    ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 *    PURPOSE OR NONINFRINGEMENT OF ANY PATENT, COPYRIGHT, TRADE 
 *    SECRET OR OTHER RIGHT.  EVANS & SUTHERLAND COMPUTER CORPORATION 
 *    DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE UNINTERRUPTED
 *    OR ERROR-FREE, OR THAT DEFECTS IN THE SOFTWARE WILL BE CORRECTED.
 *    
 *    IN NO EVENT SHALL EVANS & SUTHERLAND COMPUTER CORPORATION BE 
 *    LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL, 
 *    INDIRECT OR CONSEQUENTIAL  DAMAGES OF ANY KIND, OR ANY DAMAGES 
 *    WHATSOEVER, INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS 
 *    OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, 
 *    WHETHER OR NOT EVANS & SUTHERLAND COMPUTER CORPORATION  HAS 
 *    BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED 
 *    AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION 
 *    WITH THE POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *    
 *    US Government Users Restricted Rights 
 *    Use, duplication, or disclosure by the Government is subject 
 *    to restricted rights set forth in FAR 52.227.19(c)(2) or 
 *    subparagraph (c)(1)(ii) of the Rights in Technical Data and 
 *    Computer Software clause at DFARS 252.227-7013 and/or in 
 *    similar or successor clauses in the FAR or the DOD or NASA FAR 
 *    Supplement. Contractor/manufacturer is Evans & Sutherland Computer 
 *    Corporation, 600 Komas Drive, Salt Lake City, UT 84108. 
 *    
 *    Use of this software outside the United States is subject to 
 *    US export laws and regulations.  You agree to comply with all 
 *    such laws and regulations.
 *    
 *    This statement is made under and shall be construed and 
 *    governed by the laws of the State of Utah, without regard to 
 *    conflict of laws principles.
 * 
 */


/* #includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <GL/glut.h>

#include "td.h"


/* defines */
#define DFLT_WIN_WIDTH      500
#define DFLT_WIN_HEIGHT     500
#define DFLT_BUFFER         GLUT_DOUBLE
#define DFLT_FROZEN         GL_TRUE

/* globals */
GLint
  WinWidth       = DFLT_WIN_WIDTH,  /* current width of window */
  WinHeight      = DFLT_WIN_HEIGHT, /* current height of window */
  Buffer         = DFLT_BUFFER,     /* single/double buffered */ 
  Statue         = 0,               /* display list for statue */
  Pedestal_Top   = 0,               /* display list for top of pedestal */
  Pedestal       = 0,               /* display list for rest of pedestal */
  Texture        = 0;               /* display list for texture on pedestal */

GLfloat
  Height  = 0.0,    /* half the height of the statue -- used for translating */
  Alpha   = 0.65,   /* alpha value to blend with (shininess of surface) */
  X_Rot   = 20.0,   /* rotation about x axis */
  Z_Trans = -8.0;   /* z translation of whole scene */

GLboolean
  Frozen      = DFLT_FROZEN,  /* currently frozen (not spinning)? */
  Antialias   = GL_FALSE;     /* antialiased lines on/off */

unsigned long Timer = 0;      /* Auto-terminate after this number of msecs */

/* default filenames - may be changed with command line options */
char *StatueFile = "data/rosevase.obj";
char *TextureFile = "data/marble2.rgb";

/* functions - added to by command line options */
char *Functions   = "\0                            ";  /* default functions */


/* 
 *  text()
 *
 *    General purpose text routine.  Draws a string
 *    according to format in a stroke font at x, y
 *    after scaling it by scale in all three dimensions.
 */
void text(float x, float y, float scale, char *format, ...)
{
  va_list args;
  char buffer[200], *p;

  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  glPushMatrix();
  {
      glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
      {
	  glDisable(GL_LIGHTING);
	  glDisable(GL_TEXTURE_2D);
	  glEnable(GL_LINE_SMOOTH);
	  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	  glEnable(GL_BLEND);
	  glTranslatef(0.0, 0.0, -1.0);
	  glColor3f(1.0, 1.0, 1.0);
	  glTranslatef(x, y, 0.0);
	  glScalef(scale, scale, scale);
	  for(p = buffer; *p; p++)
	      glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
      } glPopAttrib();
  } glPopMatrix();
}

/*
 *  lighting()
 *
 *    Initialize lighting for the scene.
 */
void lighting()
{
    GLfloat ambient[]  = { 0.0, 0.0, 0.0, 1.0 };  /* get ambient from global */
    GLfloat diffuse[]  = { 1.0, 1.0, 1.0, 1.0 };  /* diffuse and specular */
    GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };  /* should be the same   */
    GLfloat global_ambient[] = { 0.8, 0.8, 0.8, 1.0 };

    /* light model */
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    /* light 0 */
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

    /* light 1 */
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
}

/*
 *  plane() 
 *
 *   Draws a plane in the XY plane
 *
 *   width  - width (x dimension) of plane
 *   height - height (y dimension) of plane
 *   strips - number of strips to divide the plane up into (tessalation)
 *   flags  - bitwise or of one of more of the following:
 *            PLANE_VERTEX  - generate vertices (default)
 *            PLANE_TEXTURE - generate texture coordinates
 *            PLANE_NORMAL  - generate normals (point in -Z -- toward viewer)
 */
#define PLANE_VERTEX   (0)
#define PLANE_TEXTURE  (1 << 1)
#define PLANE_NORMAL   (1 << 2)
void plane(float width, float height, int strips, int flags)
{
    int i, j;
    float step_across = width / (float)strips;
    float step_down   = height / (float)strips;
    float across, down, u, v;
    float step_texture = 1.0 / strips;

    u = 0.0;
    for(i = 0; i < strips; i++)
    {
	across = -(width / 2.0) + (i * step_across);
	v = 0;

        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= strips; j++)
        {
	    down = -(height / 2.0) + (j * step_down);

	    if(flags & PLANE_TEXTURE)
		glTexCoord2f(u, v);
	    if(flags & PLANE_NORMAL)
		glNormal3f(0.0, 0.0, -1.0);
	    glVertex3f(across, down, 0.0);
	    
	    if(flags & PLANE_TEXTURE)
		glTexCoord2f(u + step_texture, v);
	    if(flags & PLANE_NORMAL)
		glNormal3f(0.0, 0.0, -1.0);
	    glVertex3f(across + step_across, down, 0.0);

	    v += step_texture;
        }
        glEnd();

	u += step_texture;
    }
}

/*
 *  object()
 *
 *    Initialize all the object to be used and
 *    make appropriate display lists.
 */
void object()
{
    TDobject *object;
    
    /* read the object in */
    object = tdReadObject(StatueFile);
    if(!object) /* bail if no object - error message already sent by TD-Lib */
        exit(1);

    /* scale the object to between -1.0 and 1.0
     * and move it to (0, 0, 0)
     */
    tdSize(object, 0.0);

    /* generate normals if this
     * object doesn't have them.
     */
    if(object->num_normals == 0)
        tdGenSmoothNormals(object);

    /* generate texture vertices if
     * this object doesn't have them
     */
    if(object->num_texvertices == 0)
        tdGenLinearTexvertices(object);

    /* build a display list */
    Statue = tdGenDList(object, TD_POLYGON, TD_ALL); 

    /* grab the height for translating */
    Height = object->height / 2.0;
    /* nuke it, we don't need it anymore */
    tdDeleteObject(object);

    /* create a list for the top of the pedestal */
    Pedestal_Top = glGenLists(1);
    glNewList(Pedestal_Top, GL_COMPILE);
    {
	/* top */
	glPushMatrix();
	{
	    glColor3f(1.0, 1.0, 1.0);
	    glTranslatef(0.0, 1.0, 0.0);
	    glRotatef(-90.0, 1.0, 0.0, 0.0);
	    plane(2, 2, 10, PLANE_TEXTURE | PLANE_NORMAL);
	} glPopMatrix();

    } glEndList();

    /* create a list for the rest of the pedestal */
    Pedestal = glGenLists(1);
    glNewList(Pedestal, GL_COMPILE);
    {

	/* front */
	glPushMatrix();
	{
	    glColor3f(1.0, 0.0, 0.0);
	    glTranslatef(0.0, 0.0, 1.0);
	    plane(2, 2, 10, PLANE_TEXTURE | PLANE_NORMAL);
	} glPopMatrix();

	/* back */
	glPushMatrix();
	{
	    glColor3f(0.0, 1.0, 0.0);
	    glTranslatef(0.0, 0.0, -1.0);
	    glRotatef(180.0, 0.0, 1.0, 0.0);
	    plane(2, 2, 10, PLANE_TEXTURE | PLANE_NORMAL);
	} glPopMatrix();
	
	/* left */
	glPushMatrix();
	{
	    glColor3f(1.0, 0.0, 1.0);
	    glTranslatef(-1.0, 0.0, 0.0);
	    glRotatef(-90.0, 0.0, 1.0, 0.0);
	    plane(2, 2, 10, PLANE_TEXTURE | PLANE_NORMAL);
	} glPopMatrix();

	/* right */
	glPushMatrix();
	{
	    glColor3f(1.0, 1.0, 0.0);
	    glTranslatef(1.0, 0.0, 0.0);
	    glRotatef(90.0, 0.0, 1.0, 0.0);
	    plane(2, 2, 10, PLANE_TEXTURE | PLANE_NORMAL);
	} glPopMatrix();
	
	/* bottom */
	glPushMatrix();
	{
	    glColor3f(0.0, 0.0, 1.0);
	    glTranslatef(0.0, -1.0, 0.0);
	    glRotatef(90.0, 1.0, 0.0, 0.0);
	    plane(2, 2, 10, PLANE_TEXTURE | PLANE_NORMAL);
	} glPopMatrix();

    } glEndList();
}

/*
 *  texture()
 *
 *    Initialize all texture images needed.
 *    Creates appropriate display lists.
 */
void texture()
{
    TDteximage *image;

    image = tdReadTeximage(TextureFile);
    if(image == NULL)
	exit(-1);
    Texture = glGenLists(1);
    glNewList(Texture, GL_COMPILE_AND_EXECUTE);
    {
        /* modulate the texture for highlights */
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        /* bi-linear mipmapping */
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        /* build the mipmaps */
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image->width, image->height,
                          GL_RGB, GL_UNSIGNED_BYTE, image->data);

    } glEndList();
    free(image->data);
    free(image);
}

/*
 *  init()
 *
 *    Initializes state and display lists, etc.
 */
void init()
{
    /* depth */
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    /* lighting */
    lighting();
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    /* blending */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* culling */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    /* clearcolor */
    glClearColor(0.0, 0.0, 0.0, 0.0);

    /* texture */
    texture();

    /* object */
    object();
}

/*
 *  display()
 *
 *    Display callback routine, does all the rendering.
 */
void display()
{
    static float spin = 20.0;
    static GLfloat white[]    = { 1.0, 1.0, 1.0, 0.65 };
    static GLfloat specular[] = { 0.0, 0.0, 0.0, 1.0 };
    static GLfloat light_position_top[]    = { 0.0, 1.0, 1.0, 0.0 };   
    static GLfloat light_position_bottom[] = { 0.0, -1.0, 1.0, 0.0 }; 


    white[3] = Alpha;

    if(!Frozen)
    {
        spin += 2.0;
        if(spin > 360.0)    /* reset after full rotation - avoid an overflow */
            spin -= 360.0;
    }

    /* clear the buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    {
        /* default translation - translate in Z because
         * most everything is drawn at or around (0, 0, 0)
         */
        glTranslatef(0.0, 0.0, Z_Trans);
        
        /* position the lights */
        glLightfv(GL_LIGHT0, GL_POSITION, light_position_top);
#if 0
        glLightfv(GL_LIGHT1, GL_POSITION, light_position_bottom);
#endif
        
        /* rotation around the X axis (tilt toward plane of screen) */
        glRotatef(X_Rot, 1.0, 0.0, 0.0);        
        glRotatef(spin, 0.0, 1.0, 0.0);

        glPushMatrix();
        {
            /* move down a bit, and draw the bottom of
             * the pedestal (all but the top)
             */
	    glEnable(GL_TEXTURE_2D);
            glTranslatef(0.0, -1.0, 0.0);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, white);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
            glCallList(Texture);
            glCallList(Pedestal);
	    glDisable(GL_TEXTURE_2D);

	} glPopMatrix();

	glPushMatrix();
	{
            /* draw the REFLECTED objects - 
             * these have to be drawn upside down, hence the 
             * scale by negative one.
             */

            /* don't need to worry about the lights, because
             * the scale flubes the normals in such a way that
             * it looks okay.
             * glDisable(GL_LIGHT0);
             * glEnable(GL_LIGHT1);
             */

            /* scale by -1 in Y, so that it is upside down
             */
            glScalef(1.0, -1.0, 1.0);

            /* move up (really down because of -1 scale) a
             * bit - below the pedestal */
            glTranslatef(0.0, Height, 0.0);

            /* NOTE that culling must be off, or else we must
             * cull the FRONT faces because the scale by -1 causes
             * the front faces to be back faces.
             */
	    glCullFace(GL_FRONT);

            /* draw the statue upside down */
            glCallList(Statue);
	    glCullFace(GL_BACK);

        } glPopMatrix();

        glPushMatrix();
        {
	    /* move down a bit */
            glTranslatef(0.0, -1.0, 0.0);

            /* blend the top of the pedestal with the
             * statue - note that there must be nothing
             * in the background at this point, or that 
             * will be blended in with the statue.  It
             * is best to have a black background.
             */
            glEnable(GL_BLEND);
            
            /* draw the top of the pedestal */
            glEnable(GL_TEXTURE_2D);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, white);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
            glCallList(Texture);
            glCallList(Pedestal_Top);
            glDisable(GL_TEXTURE_2D);

            /* don't need to blend anymore */
            glDisable(GL_BLEND);

        } glPopMatrix();

        glPushMatrix();
        {
            /* draw the ACTUAL objects */

            /* move up a bit - above the pedestal */
            glTranslatef(0.0, Height, 0.0);
            
	    /* don't really need this, but some models
	     * are constructed in such a way that this is
	     * needed - such as the rose+vase model...the
	     * leaves disappear when they are backfacing
	     * if culling is left on
	     */
	    glDisable(GL_CULL_FACE);
            glCallList(Statue);
	    glEnable(GL_CULL_FACE);
            
        } glPopMatrix();
        
    } glPopMatrix();

    /* swap the buffers and flush */
    glutSwapBuffers();
}

/*
 *  idle()
 *
 *    Idle function.  Simply call the display() routine.
 */
void idle()
{
    display();
}

/*
 *  reshape()
 *
 *    Reshape callback routine.  Resets the windows
 *    viewport and matrices.  Also sets the light 
 *    positions.
 */
void reshape(int width, int height)
{
    WinWidth = width;
    WinHeight = height;

    /* reset the viewport */
    glViewport(0, 0, width, height);

    /* reset projection */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /* Putting in 1.0 for the second argument to 
     * gluPerspective() causes the aspect ratio to be
     * calculated according to the actual size of the
     * window.  If it is long and skinny, the picture
     * drawn will be long and skinny (distorted).
     * It can be made to always respect an aspect ratio
     * by putting (GLfloat)width / (GLfloat)height
     * in as the second argument to gluPerspective().
     */
    gluPerspective(30.0, 1.0, 1.0, 64.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/*
 *  function()
 *
 *    Function dispatch routine
 *
 */
void function(int func)
{
    switch(func)
    {
    case 27:                 /* Esc quits */
        exit(1);
        break;

    case 'A':                /* decrease alpha = increase shininess */
        Alpha -= 0.01;
        if(Alpha < 0.6)
            Alpha = 0.6;
        break;

    case 'a':                /* increase alpha = decrease shininess */
        Alpha += 0.01;
        if(Alpha > 1.0)
            Alpha = 1.0;
        break;

    case 'Z':                /* zoom in */
        Z_Trans += 0.25;
        if(Z_Trans > -1.75)  /* check for clipping */
            Z_Trans = -1.75;
        break;

    case 'z':                /* zoom out */
        Z_Trans -= 0.25;
        break;
        
    case 'T':                /* tilt up */
        X_Rot -= 2.0;
        break;
        
    case 't':                /* tilt down */
        X_Rot += 2.0;
        break;

    case '!':
        Frozen ? glutIdleFunc(idle) : glutIdleFunc(NULL);
        Frozen = !Frozen;
	break;

    case '\r':  /* post a redisplay on RETURN key */
	break;

    default:    /* return by default (don't post a redisplay) */
	return;
    }

    glutPostRedisplay();
}

/*
 *  item()
 *
 *    Changes a given menu item in the current menu
 */
void item(int num, char *text, char value, char key)
{
    static char string[256];

    sprintf(string, "[%c] %s (%c)", value, text, key);
    glutChangeToMenuEntry(num, string, key);
}

/*
 *  menu()
 *
 *    Send all menu requests to the function() callback.
 *    Update/initialize the menu as needed.
 */
void menu(int value)
{
    static int mainmenu = -1;
    static int alpha;
    static int tilt;
    static int zoom;
    static char string[64];

    /* send the value off for processing */
    function(value);

    if(mainmenu == -1)  /* create the menu */
    {
	alpha = glutCreateMenu(menu);
	  glutAddMenuEntry("", '\0');
  	  glutAddMenuEntry("", '\0');
	tilt = glutCreateMenu(menu);
	  glutAddMenuEntry("", '\0');
  	  glutAddMenuEntry("", '\0');
	zoom = glutCreateMenu(menu);
	  glutAddMenuEntry("", '\0');
  	  glutAddMenuEntry("", '\0');
	mainmenu = glutCreateMenu(menu);
	  glutAddMenuEntry("        Statue      ", '\0');
	  glutAddMenuEntry("                    ", '\0');
	  glutAddSubMenu(  "Pedestal Shininess  ", alpha);
	  glutAddSubMenu(  "Tilt                ", tilt);
	  glutAddSubMenu(  "Zoom                ", zoom);
	  glutAddMenuEntry("                    ", '\0');
	  glutAddMenuEntry("[ ] Freeze Frame (!)", '\0');
	  glutAddMenuEntry("                    ", '\0');
	  glutAddMenuEntry("Quit           (Esc)", 27);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
    }

    /* update the menu */
    glutSetMenu(alpha);
    sprintf(string, "Increase (A)");
    glutChangeToMenuEntry(1, string, 'A');
    sprintf(string, "Decrease (a)");
    glutChangeToMenuEntry(2, string, 'a');

    glutSetMenu(tilt);
    sprintf(string, "Up   (T)");
    glutChangeToMenuEntry(1, string, 'T');
    sprintf(string, "Down (t)");
    glutChangeToMenuEntry(2, string, 't');

    glutSetMenu(zoom);
    sprintf(string, "In   (Z)");
    glutChangeToMenuEntry(1, string, 'Z');
    sprintf(string, "Out  (z)");
    glutChangeToMenuEntry(2, string, 'z');

    glutSetMenu(mainmenu);
    item(7, "Freeze Frame", Frozen ? 'X' : ' ', '!');
}

/*
 *  keyboard()
 *
 *    Send all key requests to the menu() callback
 *    so that the menu will be updated properly.
 */
void keyboard(unsigned char key, int x, int y)
{
    menu(key);
}

/*
 *  special()
 *
 *    Send all special key requests to the menu() callback
 *    negated so that the menu will be updated properly.
 *    Special keys include non-ascii/non-printable characters.
 */
void special(int key, int x, int y)
{
    /* negate so that menu() can tell these apart
     * from the keyboard() keys
     */
    menu(-key);
}

/*
 *  timer()
 *
 *    A timer function to facilitate any function that
 *    needs to be performed at a specific time (such as
 *    terminating the program after a specified time)
 */
void timer(int value)
{
    menu(value);
}

/*
 *  menustate()
 *
 *    Turn off rendering when the menu is visible.  This
 *    eases the use of the menus if something complicated
 *    is being rendered.
 */
void menustate(int state)
{
    if(!Frozen)
	state == GLUT_MENU_IN_USE ? glutIdleFunc(NULL) : glutIdleFunc(idle);
}

/*
 *  visibility()
 *
 *    When the window isn't visible, turn off rendering
 */
void visiblity(int state)
{
    if(!Frozen)
	state == GLUT_VISIBLE ? glutIdleFunc(idle) : glutIdleFunc(NULL);
}

/*
 *  usage()
 *
 *    Shows a message on what command line
 *    options are available.  Also prints
 *    version number.
 */
void usage(
  char *format, ...   /* format e.g., "%s %s\n", "hello", "world" */
	   )
{
    va_list args;
    char buffer[256], *p;
    
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    fprintf(stderr, "\n%s version %s\n\n", PROGRAM, VERSION);
    if(strcmp(format, ""))
	fprintf(stderr, " *** ERROR - %s\n\n", buffer);
    fprintf(stderr, "usage: %s [options ...]\n\n", PROGRAM);
    fprintf(stderr, "    option     short params  "
	    "description                       default\n"
	            "    ---------- ----- ------- "
	    "--------------------------------- -------------\n");
#if !defined(WIN32)
    fprintf(stderr, "    -display         DISPLAY " 
	    "display for rendering             [%s]\n",
	    ":0");
#endif
    fprintf(stderr, "    -geometry        WxH+X+Y " 
	    "window geometry                   [%dx%d+0+0]\n",
	    WinWidth, WinHeight);
    fprintf(stderr, "    -direct                  " 
	    "direct rendering                  [X]\n");
    fprintf(stderr, "    -indirect                " 
	    "indirect rendering                [ ]\n");
    fprintf(stderr, "    -double    (-db)         " 
	    "double buffered rendering         [%c]\n",
	    Buffer == GLUT_DOUBLE ? 'X' : ' ');
    fprintf(stderr, "    -single    (-sb)         " 
	    "single buffered rendering         [%c]\n",
	    Buffer == GLUT_SINGLE ? 'X' : ' ');
    fprintf(stderr, "    -texture   (-t)  TEXFILE "
	    "file containing texture map       [%s]\n",
	    TextureFile);
    fprintf(stderr, "    -statue    (-s)  OBJFILE "
            "object to use as a statue         [%s]\n",
            StatueFile);
    fprintf(stderr, "    -functions (-f)  FUNCTNS "
	    "functions to perform on startup   [%s]\n",
	    Functions);
    fprintf(stderr, "    -autoquit  (-q)  SECONDS "
            "auto-terminate program after time [%s]\n",
	    Timer ? "enabled" : "disabled");
    fprintf(stderr, "\n");
#if !defined(WIN32)
    fprintf(stderr, "        DISPLAY - connection to an X server\n");
#endif
    fprintf(stderr, "        WxH+X+Y - geometry specification (width by "
	    "height + x pos + y pos)\n");
    fprintf(stderr, "        TEXFILE - file that contains a texture map\n");
    fprintf(stderr, "        OBJFILE - file that contains an object\n");
    fprintf(stderr, "        SECONDS - number of seconds to wait before "
	    "program auto-termination\n");
    fprintf(stderr, "        FUNCTNS - a list of functions to perform:\n");
    fprintf(stderr, "                  A  Increase Pedestal Shininess\n");
    fprintf(stderr, "                  a  Decrease Pedestal Shininess\n");
    fprintf(stderr, "                  T  Tilt up\n");
    fprintf(stderr, "                  t  Tilt down\n");
    fprintf(stderr, "                  Z  Zoom in\n");
    fprintf(stderr, "                  z  Zoom out\n");
    fprintf(stderr, "                  !  Freeze frame toggle\n");
    fprintf(stderr, "\n");
    exit(1);
}
    
/*
 *  parse()
 *
 *    Parses the command line arguments and sets
 *    appropriate conditions for startup.
 */
void parse(
  int argc,    /* argc passed into main() */
  char **argv  /* argv passed into main() */
	   )
{
    while(--argc)
    {
	++argv;
	
	if(!strcmp("-help", argv[0]) || !strcmp("-h", argv[0]))
        {
            usage("");
        }
#if GLUT_TAKES_CARE_OF_THESE_COMMAND_LINE_OPTIONS
        else if(!strcmp("-display", argv[0]))
        {
            if(argc == 1)
                usage("Option `%s' requires an argument!", argv[0]);
            --argc; ++argv;
            /* GLUT takes care of this argument */
        }
        else if(!strcmp("-geometry", argv[0]))
        {
            if(argc == 1)
                usage("Option `%s' requires an argument!", argv[0]);
            --argc; ++argv;
            /* GLUT takes care of this argument */
        }
        else if(!strcmp("-direct", argv[0]))
        {
            /* GLUT takes care of this argument */
        }
        else if(!strcmp("-indirect", argv[0]))
        {
            /* GLUT takes care of this argument */
        }
#endif
        else if(!strcmp("-double", argv[0]) || !strcmp("-db", argv[0]))
	{
	    Buffer = GLUT_DOUBLE;
	}
        else if(!strcmp("-single", argv[0]) || !strcmp("-sb", argv[0]))
        {
            Buffer = GLUT_SINGLE;
        }
        else if(!strcmp("-texture", argv[0]) || !strcmp("-t", argv[0]))
        {
            if(argc == 1)
                usage("Option `%s' requires an argument!", argv[0]);
            else
                TextureFile = argv[1];
            --argc; ++argv;
        }
        else if(!strcmp("-statue", argv[0]) || !strcmp("-s", argv[0]))
        {
            if(argc == 1)
                usage("Option `%s' requires an argument!", argv[0]);
            else
                StatueFile = argv[1];
            --argc; ++argv;
	}
        else if(!strcmp("-autoquit", argv[0]) || !strcmp("-q", argv[0]))
        {
            if(argc == 1)
                usage("Option `%s' requires an argument!", argv[0]);
            else
                sscanf(argv[1], "%d", &Timer);
            --argc; ++argv;
        }
        else if(!strcmp("-functions", argv[0]) || !strcmp("-f", argv[0]))
        {
            if(argc == 1)
                usage("Option `%s' requires an argument!", argv[0]);
            else
		sscanf(argv[1], "%s", Functions);
            --argc; ++argv;
        }
	else
	    usage("Invalid option `%s'!", argv[0]);
    }
}

/*
 *  main()
 */
void main(int argc, char **argv)
{
    glutInitWindowSize(WinWidth, WinHeight);
    glutInit(&argc, argv);

    parse(argc, argv);

    glutInitDisplayMode(GLUT_RGB | Buffer | GLUT_DEPTH);
    glutCreateWindow("Statue");

    init();

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutVisibilityFunc(visiblity);
    glutMenuStateFunc(menustate);
    if(Timer)
	glutTimerFunc(Timer * 1000, timer, 27);

    /* initialize the menu */
    menu('\0');

    /* execute the default functions */
    while(*Functions)
    {
	menu(*Functions);
	Functions++;
    }

    /* do an initial reshape */
    reshape(WinWidth, WinHeight);

    glutMainLoop();
}

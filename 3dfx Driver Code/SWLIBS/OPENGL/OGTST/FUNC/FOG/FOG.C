/**************************************************************************
 *									  *
 * 		 Copyright (C) 1993, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/* fog.c - $Revision: 2$ */

#include "math.h"	
#include "ogtst.h"	/* include test environment		*/

static float ner,myfar;
static GLint  fogmode;
static float fogdensity;
static float fognear;
static float fogfar;
static float fogcolor[4];
static float objcol[4];
static int   primtype;
static int   texturing;

/* fog error tolerance table */
static int fog_tolerance[9][3] = {
/***** EXP  EXP2  LINEAR ***************/
        0,    0,    0,          /* GENERIC  */
};

static void fogcheck(float *);
static void select_fog(GLint);

TESTMOD(fog)
{
   GLint j;
   float vx[4],r[4];
   GLuint h;

   glMatrixMode(GL_PROJECTION) ;

   glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   h = 0xffffffff;
   glTexImage2D(GL_TEXTURE_2D,0,4,1,1,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,&h);
   glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

   glPointSize(3);
   glLineWidth(3);

   glClearColor(0,0,0,0);
   glClear(GL_COLOR_BUFFER_BIT);

   while (pass--) {
     ner = ogLibFloatRand(-10.0,-0.1);
     myfar =  -ner;

     if (ogLibBitRand(1))  {
       glLoadIdentity();
       glOrtho(0.0,ogEnvQuery(OG_XWSIZE),0.0,ogEnvQuery(OG_YWSIZE),ner,myfar);
       select_fog(ogLibIntRand(0,2));
       ogEnvLog(2,"matrix before fogvertex: %f\n",ner);
       }
     else  {
       select_fog(ogLibIntRand(0,2));
       glLoadIdentity();
       glOrtho(0.0,ogEnvQuery(OG_XWSIZE),0.0,ogEnvQuery(OG_YWSIZE),ner,myfar);
       ogEnvLog(2,"fogvertex before matrix: %f\n",ner);
       }

     for (j=0; j<32; j++) {
	  vx[0] = ogLibIntRand(3,ogEnvQuery(OG_XWSIZE)-3);
	  vx[1] = ogLibIntRand(3,ogEnvQuery(OG_YWSIZE)-3);
	  vx[2] = ogLibFloatRand(ner,0.0);

	  objcol[0] = ogLibFloatRand(0.0,1.0);
	  objcol[1] = ogLibFloatRand(0.0,1.0); 
	  objcol[2] = ogLibFloatRand(0.0,1.0);
	  objcol[3] = ogLibFloatRand(0.0,1.0);
          
          /*
          ** test both textured and non-textured fog.
          */
	  texturing = ogLibBitRand(1);
	  if (texturing)
	     glEnable(GL_TEXTURE_2D);
	  else
	     glDisable(GL_TEXTURE_2D);

          /*
          ** test points,lines,and tris
          */
	  glColor4f(objcol[0],objcol[1],objcol[2],objcol[3]);

 	  primtype = ogLibIntRand(0,2);
 	  switch(primtype) {
	     case 0:
#if 1
		glBegin(GL_POINTS);
		glVertex3fv(vx);
		glEnd();
		ogEnvLog(2,"GL_POINTS\n");
		break;
#endif
	     case 1:
		glBegin(GL_LINES);
		r[0] = vx[0]-1; r[1] = vx[1]; r[2] = vx[2]; glVertex3fv(r);
		r[0] = vx[0]+1; r[1] = vx[1]; r[2] = vx[2]; glVertex3fv(r);
		glEnd();
		ogEnvLog(2,"GL_LINES\n");
		break;
	     case 2:
		glBegin(GL_POLYGON);
		r[0] = vx[0]-1; r[1] = vx[1]-1; r[2] = vx[2]; glVertex3fv(r);
		r[0] = vx[0]+1; r[1] = vx[1]-1; r[2] = vx[2]; glVertex3fv(r);
		r[0] = vx[0]+1; r[1] = vx[1]+1; r[2] = vx[2]; glVertex3fv(r);
		r[0] = vx[0]-1; r[1] = vx[1]+1; r[2] = vx[2]; glVertex3fv(r);
		glEnd();
		ogEnvLog(2,"GL_POLYGON\n");
		break;
	     default:
		ogEnvLog(OG_LINTERNALERROR,"invalid primitive type %d\n",primtype);
		break;
	  }

          fogcheck(vx);
          }

      glDisable(GL_FOG);
      }
}

static void fogcheck(float vx[4])
{
   float f;
   GLuint buffer;
   GLint col[4],tol;
   GLint mm;
   GLint color;

   switch (fogmode) {
     case 0:            /* GL_EXP */
       f = 255.0 * exp(fogdensity * vx[2]);
       break;

     case 1:            /* GL_EXP2 */
       f = 255.0 * exp(-fogdensity * fogdensity * vx[2] * vx[2]);
       break;

     case 2:            /* GL_LINEAR */
       f = 255.0 * (fogfar + vx[2]) / (fogfar - fognear);
       break;

     default:
       ogEnvLog(OG_LINTERNALERROR,"fogmode = %d\n",fogmode);
       break;
     }

   ogEnvLog(2,"f = %f, z = %f\n",f/255.0,vx[2]);

   col[0] = f * objcol[0] + (255.0-f)*fogcolor[0];
   col[1] = f * objcol[1] + (255.0-f)*fogcolor[1];
   col[2] = f * objcol[2] + (255.0-f)*fogcolor[2];
   col[3] = 255.0 * objcol[3];

   buffer = 0xdeadbeef;
   ogLibReadPixels(vx[0],vx[1],vx[0],vx[1],&buffer);

   mm = ogEnvCurVisualInfo(GLX_ALPHA_SIZE);
   if (mm <= 0)  {
     col[3] = 255;
     buffer |= 255;
     }

   color = (col[0]<<24) | (col[1]<<16) | (col[2]<<8) | col[3];

   switch (ogEnvQuery(OG_HW))  {
      case  OG_GENERIC : tol = fog_tolerance[0][fogmode] ; break ;
      case  OG_VG1 : tol = fog_tolerance[0][fogmode] ; break ;
      default: ogEnvLog(OG_LINTERNALERROR,"unsupported hardware\n"); break;
      }

   ogLibColCheck(color, buffer, tol);
}

CLEANUP(fog)
{
   float junk[4];
   
   glDisable(GL_FOG);
   glDisable(GL_TEXTURE_2D);
   junk[0] = junk[1] = junk[2] = junk[3] = 0;
   glFogfv(GL_FOG_COLOR, junk);
   glFogf(GL_FOG_DENSITY, 1);
   glFogf(GL_FOG_START, 0);
   glFogf(GL_FOG_END, 1);
   glFogf(GL_FOG_MODE, GL_EXP);
   glPointSize(1);
   glLineWidth(1);

   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW) ;

   ogLibSetDefaultTextures();
   ogLibSetDefaultColors();
}

static void select_fog(GLint mode)
{
   fogmode = mode;

   ogEnvLog(1,"fogmode = %d\n",fogmode);

   switch (fogmode) {
     case 0:            /* GL_EXP */
       fogdensity  = ogLibFloatRand(0.0,4.0);
       glFogf(GL_FOG_DENSITY,fogdensity);

       glFogf(GL_FOG_MODE,GL_EXP);
       break;

     case 1:            /* GL_EXP2 */
       fogdensity  = ogLibFloatRand(0.0,4.0);
       glFogf(GL_FOG_DENSITY,fogdensity);

       glFogf(GL_FOG_MODE,GL_EXP2);
       break;

     case 2:            /* GL_LINEAR */
       fognear = ner;
       glFogf(GL_FOG_START,fognear);

       fogfar  = myfar;
       glFogf(GL_FOG_END,fogfar);

       glFogf(GL_FOG_MODE,GL_LINEAR);
       break;

     default:
       ogEnvLog(OG_LINTERNALERROR,"fogmode = %d\n",fogmode);
       break;
     }

   fogcolor[0] = ogLibFloatRand(0.0,1.0);
   fogcolor[1] = ogLibFloatRand(0.0,1.0);
   fogcolor[2] = ogLibFloatRand(0.0,1.0);
   fogcolor[3] = 1.0;
   glFogfv(GL_FOG_COLOR,fogcolor);

   glEnable(GL_FOG);
}


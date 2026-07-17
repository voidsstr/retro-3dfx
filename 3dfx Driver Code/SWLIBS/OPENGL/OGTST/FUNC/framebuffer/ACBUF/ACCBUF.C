/*########################################################################
#                                                                        #
#                Copyright (C) 1996, Silicon Graphics, Inc.              #
#                                                                        #
#  These coded instructions, statements, and computer programs  contain  #
#  unpublished  proprietary  information of Silicon Graphics, Inc., and  #
#  are protected by Federal copyright law.  They  may  not be disclosed  #
#  to  third  parties  or copied or duplicated in any form, in whole or  #
#  in part, without the prior written consent of Silicon Graphics, Inc.  #
#                                                                        #
########################################################################*/

/*
 * $Revision: 2$
 */

#include "ogtst.h"

#define AC_ACBUF_CLEAR	0xdead0000
#define AC_ACCUM	0xdead0001
#define AC_LOAD		0xdead0002
#define AC_RETURN	0xdead0003
#define AC_UNIT_RETURN	0xdead0004
#define AC_MULT		0xdead0005
#define AC_ADD		0xdead0006
#define AC_COLOR_CLEAR	0xdead0007

static int   acc_tol,xmax,ymax ;
static float acc_color[4] ;
static float scr_color[4] ;
static float mask[4];

static void rectcheck(void);
static void ogtst_acbuf(long);

TESTMOD(accbuf)
{
    long  i;

    acc_tol = 2;

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;
    mask[0] = ogEnvCurVisualInfo(GLX_RED_SIZE) ? 1 : 0;
    mask[1] = ogEnvCurVisualInfo(GLX_GREEN_SIZE) ? 1 : 0;
    mask[2] = ogEnvCurVisualInfo(GLX_BLUE_SIZE) ? 1 : 0;
    mask[3] = ogEnvCurVisualInfo(GLX_ALPHA_SIZE) ? 1 : 0;

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearAccum(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glOrtho(-1.0,1.0,-1.0,1.0,1.0,-1.0);

    ogEnvLog(1,"--- CLEAR test ---\n") ;
    for (i=0; i<pass; i++)  {
      ogtst_acbuf(AC_ACBUF_CLEAR) ;
      ogtst_acbuf(AC_UNIT_RETURN) ;
      }

    ogEnvLog(1,"--- ACCUM test ---\n") ;
    for (i=0; i<pass; i++)  {
      ogtst_acbuf(AC_COLOR_CLEAR) ;
      ogtst_acbuf(AC_ACBUF_CLEAR) ;
      ogtst_acbuf(AC_ACCUM) ;
      ogtst_acbuf(AC_UNIT_RETURN) ;
      }

    ogEnvLog(1,"--- LOAD test ---\n") ;
    for (i=0; i<pass; i++)  {
      ogtst_acbuf(AC_COLOR_CLEAR) ;
      ogtst_acbuf(AC_LOAD) ;

      glClearColor(0.0, 0.0, 0.0, 0.0);
      glClear(GL_COLOR_BUFFER_BIT);

      ogtst_acbuf(AC_UNIT_RETURN) ;
      }

    ogEnvLog(1,"--- ADD test ---\n") ;
    for (i=0; i<pass; i++)  {
      ogtst_acbuf(AC_ACBUF_CLEAR) ;
      ogtst_acbuf(AC_ADD) ;
      ogtst_acbuf(AC_UNIT_RETURN) ;
      }

    ogEnvLog(1,"--- MULT test ---\n") ;
    for (i=0; i<pass; i++)  {
      ogtst_acbuf(AC_ACBUF_CLEAR) ;
      ogtst_acbuf(AC_MULT) ;
      ogtst_acbuf(AC_UNIT_RETURN) ;
      }

    ogEnvLog(1,"--- RETURN test ---\n") ;
    for (i=0; i<pass; i++)  {
      ogtst_acbuf(AC_ACBUF_CLEAR) ;
      ogtst_acbuf(AC_RETURN) ;
      }
}

CLEANUP(accbuf)
{
    glClearAccum(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
}

/*************************************************************
*  rectcheck()  -
*************************************************************/
static void rectcheck(void)

{  int i,x,y ;
   int r,g,b,a,scr[4] ;
   unsigned int buffer,col ;

   for (i=0; i<16; i++)   {
     x = ogLibIntRand(0,xmax);
     y = ogLibIntRand(0,ymax);
     ogEnvLog(2,"(x,y) = (%d,%d)\n",x,y) ;

     scr[0] = scr_color[0]*mask[0];
     scr[1] = scr_color[1]*mask[1];
     scr[2] = scr_color[2]*mask[2];
     scr[3] = mask[3]!=0 ? scr_color[3] : 0xff;
     col = (scr[0]<<24) | (scr[1]<<16) | (scr[2]<<8) | (scr[3]);

     ogLibReadPixels(x,y,x,y,&buffer);

     a = (buffer      ) & 0xFF ;
     b = (buffer >>  8) & 0xFF ;
     g = (buffer >> 16) & 0xFF ;
     r = (buffer >> 24) & 0xFF ;

     if (ABS(r-scr[0]) > acc_tol || ABS(g-scr[1]) > acc_tol ||
         ABS(b-scr[2]) > acc_tol || ABS(a-scr[3]) > acc_tol)
       ogEnvLog(OG_LFAIL,"color is 0x%08x but should be 0x%08x\n",buffer,col);
     }
}

/*************************************************************
*  ogtst_acbuf() -
*************************************************************/
static void ogtst_acbuf(long op)

{  int i ;
   float value,varr[4];

   switch (op)  {
     case AC_ACCUM:
            value = ogLibFloatRand(-0.5,0.5) ; 
            glAccum(GL_ACCUM,value);
            ogEnvLog(1,"glAccum(GL_ACCUM,%f);\n",value) ;

            for (i=0; i<4; i++)  
              acc_color[i] += value*scr_color[i];
            break ;

     case AC_LOAD:
            value = ogLibFloatRand(-0.5,0.5) ; 
            glAccum(GL_LOAD,value);
            ogEnvLog(1,"glAccum(GL_LOAD,%f);\n",value) ;

            for (i=0; i<4; i++)  
              acc_color[i] = value*scr_color[i];
            break ;
 
     case AC_MULT:
            value = ogLibFloatRand(-0.5,0.5) ; 
            glAccum(GL_MULT,value);
            ogEnvLog(1,"glAccum(GL_MULT,%f);\n",value) ;

            for (i=0; i<4; i++)
              acc_color[i] = value*acc_color[i];
            break ;

     case AC_ADD:
            value = ogLibFloatRand(-0.5,0.5) ; 
            glAccum(GL_ADD,value);
            ogEnvLog(1,"glAccum(GL_MULT,%f);\n",value) ;

            for (i=0; i<4; i++)
              acc_color[i] += value;
            break ;

     case AC_UNIT_RETURN:
            glAccum(GL_RETURN,1.0);
            ogEnvLog(1,"glAccum(GL_RETURN,1.0);\n") ;

            for (i=0; i<4; i++)  {
               scr_color[i] = 255.0*acc_color[i];
               if (scr_color[i] > 255) scr_color[i] = 255;
               if (scr_color[i] <   0) scr_color[i] =   0;
               }

            rectcheck() ;
            break ;

     case AC_RETURN:
            value = ogLibFloatRand(-2.0,2.0) ; 
            glAccum(GL_RETURN,value);
            ogEnvLog(1,"glAccum(GL_RETURN,%f);\n",value) ;

            for (i=0; i<4; i++)  {
               scr_color[i] = 255.0*value*acc_color[i];
               if (scr_color[i] > 255) scr_color[i] = 255; 
               if (scr_color[i] <   0) scr_color[i] =   0; 
               }

            rectcheck() ;
            break ;

     case AC_ACBUF_CLEAR:
            varr[0] = ogLibFloatRand(-0.5,0.5);
            varr[1] = ogLibFloatRand(-0.5,0.5);
            varr[2] = ogLibFloatRand(-0.5,0.5);
            varr[3] = ogLibFloatRand(-0.5,0.5);
            glClearAccum(varr[0],varr[1],varr[2],varr[3]);
            ogEnvLog(1,"glClearAccum(%f,%f,%f,%f);\n",varr[0],varr[1],varr[2],varr[3]);
            glClear(GL_ACCUM_BUFFER_BIT);
            ogEnvLog(1,"glClear(GL_ACCUM_BUFFER_BIT);\n",value) ;

            for (i=0; i<4; i++)
              acc_color[i] = varr[i];
            break ;

     case AC_COLOR_CLEAR:
            varr[0] = ogLibFloatRand(0.0,1.0);
            varr[1] = ogLibFloatRand(0.0,1.0);
            varr[2] = ogLibFloatRand(0.0,1.0);
            varr[3] = ogLibFloatRand(0.0,1.0);
            glClearColor(varr[0],varr[1],varr[2],varr[3]);
            ogEnvLog(1,"glClearColor(%f,%f,%f,%f);\n",varr[0],varr[1],varr[2],varr[3]);
            glClear(GL_COLOR_BUFFER_BIT);
            ogEnvLog(1,"glClear(GL_COLOR_BUFFER_BIT);\n");

            for (i=0; i<4; i++)
              scr_color[i] = varr[i]; 
            break ;

     default :
            ogEnvLog(OG_LINTERNALERROR,"accumulation buffer simulation error\n") ;
            break ;
    }
}


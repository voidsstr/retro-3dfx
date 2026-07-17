/**************************************************************************
 *									  *
 * 	 Copyright (C) 1987, 1988 Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

#include "ogtst.h"
#include "light.h"
#include <math.h> 

TESTMOD(light) 

{  int i,loop,flag,count,mask;
   int spec_vers,Xsize,Ysize,shine;
   unsigned int retcol;

   Xsize = ogEnvQuery(OG_XWSIZE);
   Ysize = ogEnvQuery(OG_YWSIZE);

   glMatrixMode(GL_PROJECTION) ;
   glLoadIdentity() ;
   glOrtho(-0.5,Xsize-0.5,-0.5,Ysize-0.5,-1024.0,1024.0);

   glMatrixMode(GL_MODELVIEW) ;
   glLoadIdentity() ;

   glShadeModel(GL_FLAT);
   glEnable(GL_NORMALIZE);
   glEnable(GL_LIGHTING);
   glPolygonMode(GL_FRONT_AND_BACK,GL_POINT) ;

   for (spec_vers=0; spec_vers<3; spec_vers++)  {
     switch (spec_vers)  {
       case  0 : mask = ~OGTST_MATERIAL_SPECULAR & ~OGTST_LIGHT_SPOT_CUTOFF;
                 ogEnvLog(1, "TEST WITHOUT SPECULAR REFLECTION AND SPOTLIGHTS\n") ;
                 break ;
       case  1 : mask = ~OGTST_LIGHT_SPOT_CUTOFF;
                 ogEnvLog(1, "TEST WITHOUT SPOTLIGHTS\n") ;
                 break ;
       case  2 : mask = 0xFFFFFFFF;
                 ogEnvLog(1, "TEST EVERYTHING BUT THE KITCHEN SINK\n") ;
                 break ;
       }
       
     for (loop=pass; loop>0; loop--)  { 
       reset_all() ;

       glClearColor(0.0,0.0,0.0,0.0);
       glClear(GL_COLOR_BUFFER_BIT) ;
 
       count = 1 + ogLibBitRand(5) ;
 
       for (i=0; i<count; i++)  { 
         flag = mask & ogLibBitRand(32) & OGTST_DEBUG_MASK ;

         /* disable/enable */
         shine = ogLibBitRand(3);
         if (ogLibBitRand(2) == 3)
           disable(shine);
         else
           enable(shine);
           
         eye_init() ;
         object_init(flag) ;
  
         ogtst_lmgen(OGTST_LIGHT,shine,flag); 
         ogtst_lmgen(OGTST_MATERIAL,ogLibIntRand(0,2),flag) ; 
         ogtst_lmgen(OGTST_MODEL,0,flag) ; 

         retcol = LightVertex(flag) ;
  
         dumpster(retcol,spec_vers,flag) ;
         }
       }
     }
}

CLEANUP(light)
{
   reset_all() ;

   glNormal3f(0.0,0.0,1.0);

   glMatrixMode(GL_PROJECTION) ;
   glLoadIdentity() ;
   glMatrixMode(GL_MODELVIEW) ;
   glLoadIdentity() ;

   glShadeModel(GL_SMOOTH);
   glDisable(GL_NORMALIZE);
   glDisable(GL_LIGHTING);
   glPolygonMode(GL_FRONT_AND_BACK,GL_FILL) ;
}

